/*

Copyright (C) 2009, 2010  Gaetan Guidet

This file is part of gcore.

gcore is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at
your option) any later version.

gcore is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
USA.

*/

#include <gcore/pipe.h>
#include <gcore/platform.h>

gcore::PipeID gcore::Pipe::StdInID()
{
#ifdef _WIN32
  return GetStdHandle(STD_INPUT_HANDLE);
#else
  return fileno(stdin);
#endif
}

gcore::PipeID gcore::Pipe::StdOutID()
{
#ifdef _WIN32
  return GetStdHandle(STD_OUTPUT_HANDLE);
#else
  return fileno(stdout);
#endif
}

gcore::PipeID gcore::Pipe::StdErrID()
{
#ifdef _WIN32
  return GetStdHandle(STD_ERROR_HANDLE);
#else
  return fileno(stderr);
#endif
}

gcore::Pipe::Pipe()
  : mOwn(false)
#ifdef _WIN32
  , mConnected(false)
#endif
{
  mDesc[0] = INVALID_PIPE;
  mDesc[1] = INVALID_PIPE;
}

gcore::Pipe::Pipe(gcore::PipeID rid, gcore::PipeID wid)
  : mOwn(false)
#ifdef _WIN32
  , mConnected(false)
#endif
{
  mDesc[0] = rid;
  mDesc[1] = wid;
}

gcore::Pipe::Pipe(const gcore::Pipe &rhs)
  : mOwn(rhs.mOwn)
  , mName(rhs.mName)
#ifdef _WIN32
  , mConnected(rhs.mConnected)
#endif
{
  mDesc[0] = rhs.mDesc[0];
  mDesc[1] = rhs.mDesc[1];
  rhs.mOwn = false;
}

gcore::Pipe::~Pipe() {
  // Don't close pipes (leave responsability to client)
}

gcore::Pipe& gcore::Pipe::operator=(const gcore::Pipe &rhs) {
  if (this != &rhs) {
    mOwn = rhs.mOwn;
    mName = rhs.mName;
    mDesc[0] = rhs.mDesc[0];
    mDesc[1] = rhs.mDesc[1];
#ifdef _WIN32
    mConnected = rhs.mConnected;
#endif
    rhs.mOwn = false;
  }
  return *this;
}

gcore::PipeID gcore::Pipe::readID() const {
  return mDesc[0];
}

gcore::PipeID gcore::Pipe::writeID() const
{
  return mDesc[1];
}

void gcore::Pipe::close() {
  if (isNamed() && isOwned()) {
#ifndef _WIN32
    gcore::String path = "/tmp/" + mName;
    closeRead();
    closeWrite();
    unlink(path.c_str());
#else
    if (mConnected) {
      FlushFileBuffers(mDesc[1]);
      DisconnectNamedPipe(mDesc[0]);
    }
    closeRead();
    closeWrite();
#endif
  } else {
    closeRead();
    closeWrite();
  }
  mName = "";
  mOwn = false;
#ifdef _WIN32
  mConnected = false;
#endif
}

bool gcore::Pipe::open(const gcore::String &name) {
  close();
#ifndef _WIN32
  gcore::String path = "/tmp/" + name;
  int fd = ::open(path.c_str(), O_RDWR);
  if (fd != -1) {
    mName = name;
    mDesc[0] = fd;
    mDesc[1] = mDesc[0];
    return true;
  }
#else
  gcore::String pipename = "\\\\.\\pipe\\" + name;
  HANDLE hdl = CreateFile(pipename.c_str(), GENERIC_READ | GENERIC_WRITE,
                          0, NULL, OPEN_EXISTING, 0, NULL);
  if (hdl != INVALID_HANDLE_VALUE) {
    mName = name;
    mDesc[0] = hdl;
    mDesc[1] = mDesc[0];
    return true;
  }
#endif
  return false;
}

bool gcore::Pipe::create() {
  close();
#ifndef _WIN32
  if (pipe(mDesc) == -1)
#else
  SECURITY_ATTRIBUTES sattr;
  sattr.nLength = sizeof(sattr);
  sattr.lpSecurityDescriptor = NULL;
  sattr.bInheritHandle = TRUE;
  if (!CreatePipe(&mDesc[0], &mDesc[1], &sattr, 0))
#endif
  {
    mDesc[0] = INVALID_PIPE;
    mDesc[1] = INVALID_PIPE;
    return false;
  } else {
    mOwn = true;
    return true;
  }
}

bool gcore::Pipe::create(const gcore::String &name) {
  close();
#ifndef _WIN32
  gcore::String path = "/tmp/" + name;
  if (mkfifo(path.c_str(), 0666) == 0) {
    int fd = ::open(path.c_str(), O_RDWR);
    if (fd != -1) {
      mOwn = true;
      mName = name;
      mDesc[0] = fd;
      mDesc[1] = mDesc[0];
      return true;
    }
  }
#else
  // Note: May want to expose in/out buffer size
  gcore::String pipename = "\\\\.\\pipe\\" + name;
  HANDLE hdl = CreateNamedPipe(pipename.c_str(), PIPE_ACCESS_DUPLEX,
                               PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                               1, 4096, 4096, 0, NULL);
  if (hdl != INVALID_HANDLE_VALUE) {
    mOwn = true;
    mName = name;
    mConnected = false;
    mDesc[0] = hdl;
    mDesc[1] = mDesc[0];
    return true;
  }
#endif
  return false;
}

bool gcore::Pipe::isNamed() const {
  return (mName.length() > 0);
}

const gcore::String& gcore::Pipe::getName() const {
  return mName;
}

bool gcore::Pipe::isOwned() const {
  return mOwn;
}

bool gcore::Pipe::canRead() const {
  return (IsValidPipeID(mDesc[0]));
}

bool gcore::Pipe::canWrite() const {
  return (IsValidPipeID(mDesc[1]));
}

void gcore::Pipe::closeRead() {
  if (canRead()) {
#ifndef _WIN32
    ::close(mDesc[0]);
#else
    CloseHandle(mDesc[0]);
#endif
    if (mDesc[1] == mDesc[0]) {
      mDesc[1] = INVALID_PIPE;
    }
    mDesc[0] = INVALID_PIPE;
  }
}

void gcore::Pipe::closeWrite() {
  if (canWrite()) {
#ifndef _WIN32
    ::close(mDesc[1]);
#else 
    CloseHandle(mDesc[1]);
#endif
    if (mDesc[0] == mDesc[1]) {
      mDesc[0] = INVALID_PIPE;
    }
    mDesc[1] = INVALID_PIPE;
  }
}

int gcore::Pipe::read(char *buffer, int size, bool retryOnInterrupt) const {
  if (canRead()) {
#ifndef _WIN32
    int bytesRead = ::read(mDesc[0], buffer, size);
    while (bytesRead == -1 && (errno == EAGAIN || (errno == EINTR && retryOnInterrupt))) {
      bytesRead = ::read(mDesc[0], buffer, size);
    }
    return bytesRead;
#else
    bool namedPipeServer = (isNamed() && isOwned());
    if (namedPipeServer && !mConnected) {
      if (ConnectNamedPipe(mDesc[0], NULL)) {
        mConnected = true;
      } else {
        return -1;
      }
    }
    DWORD bytesRead = 0;
    BOOL rv = ReadFile(mDesc[0], buffer, size, &bytesRead, NULL);
    while (rv == FALSE) {
      DWORD lastErr = GetLastError();
      if (lastErr == ERROR_IO_PENDING) {
        rv = ReadFile(mDesc[0], buffer, size, &bytesRead, NULL);
      } else {
        if (namedPipeServer) {
          DisconnectNamedPipe(mDesc[0]);
          mConnected = false;
        }
        if (lastErr == ERROR_HANDLE_EOF || lastErr == ERROR_BROKEN_PIPE) {
          rv = TRUE;
          bytesRead = 0;
        }
        break;
      }
    }
    if (rv) {
      return bytesRead;
    }
#endif
  }
  return -1;
}

int gcore::Pipe::read(String &str, bool retryOnInterrupt) const {
  char rdbuf[256];

  if (canRead()) {
#ifndef _WIN32
    int bytesRead = ::read(mDesc[0], rdbuf, 255);
    while (bytesRead == -1 && (errno == EAGAIN || (errno == EINTR && retryOnInterrupt))) {
      bytesRead = ::read(mDesc[0], rdbuf, 255);
    }
    if (bytesRead >= 0) {
      rdbuf[bytesRead] = '\0';
      str = rdbuf;
    }
    return bytesRead;
#else
    bool namedPipeServer = (isNamed() && isOwned());
    if (namedPipeServer && !mConnected) {
      if (ConnectNamedPipe(mDesc[0], NULL)) {
        mConnected = true;
      } else {
        return -1;
      }
    }
    DWORD bytesRead = 0;
    BOOL rv = ReadFile(mDesc[0], rdbuf, 255, &bytesRead, NULL);
    while (rv == FALSE) {
      DWORD lastErr = GetLastError();
      if (lastErr == ERROR_IO_PENDING) {
        rv = ReadFile(mDesc[0], rdbuf, 255, &bytesRead, NULL);
      } else {
        if (namedPipeServer) {
          DisconnectNamedPipe(mDesc[0]);
          mConnected = false;
        }
        if (lastErr == ERROR_HANDLE_EOF || lastErr == ERROR_BROKEN_PIPE) {
          rv = TRUE;
          bytesRead = 0;
        }
        break;
      }
    }
    if (rv) {
      rdbuf[bytesRead] = '\0';
      str = rdbuf;
      return bytesRead;
    }
#endif
  }

  str = "";
  return -1;
}

int gcore::Pipe::write(const char *buffer, int size) const {
  if (canWrite()) {
#ifndef _WIN32
    int bytesToWrite = size;
    int rv = ::write(mDesc[1], buffer, bytesToWrite);
    while (rv == -1 && errno == EAGAIN) {
      rv = ::write(mDesc[1], buffer, bytesToWrite);
    }
    return rv;
#else
    bool namedPipeServer = (isNamed() && isOwned());
    if (namedPipeServer && !mConnected) {
      if (ConnectNamedPipe(mDesc[1], NULL)) {
        mConnected = true;
      } else {
        return -1;
      }
    }
    DWORD bytesToWrite = (DWORD)size;
    DWORD bytesWritten = 0;
    BOOL rv = WriteFile(mDesc[1], buffer, bytesToWrite, &bytesWritten, NULL);
    while (rv == FALSE) {
      DWORD lastErr = GetLastError();
      if (lastErr == ERROR_IO_PENDING) {
        rv = WriteFile(mDesc[1], buffer, bytesToWrite, &bytesWritten, NULL);
      } else {
        if (namedPipeServer) {
          DisconnectNamedPipe(mDesc[1]);
          mConnected = false;
        }
        break;
      }
    }
    return (rv == TRUE ? bytesWritten : -1);
#endif
  }
return -1;
}

int gcore::Pipe::write(const String &str) const {
  return write(str.c_str(), int(str.length()));
}

