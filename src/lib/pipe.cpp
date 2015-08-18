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

gcore::Pipe::Pipe() {
  mDesc[0] = INVALID_PIPE;
  mDesc[1] = INVALID_PIPE;
}

gcore::Pipe::Pipe(gcore::PipeID rid, gcore::PipeID wid) {
  mDesc[0] = rid;
  mDesc[1] = wid;
}

gcore::Pipe::Pipe(const gcore::Pipe &rhs) {
  mDesc[0] = rhs.mDesc[0];
  mDesc[1] = rhs.mDesc[1];
}

gcore::Pipe::~Pipe() {
  // Don't close pipe, leave responsability to client
  //close();
}

gcore::Pipe& gcore::Pipe::operator=(const gcore::Pipe &rhs) {
  if (this != &rhs) {
    mDesc[0] = rhs.mDesc[0];
    mDesc[1] = rhs.mDesc[1];
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
  closeRead();
  closeWrite();
}

void gcore::Pipe::create() {
  close();
#ifndef _WIN32
  if (pipe(mDesc) == -1) {
#else
  SECURITY_ATTRIBUTES sattr;
  sattr.nLength = sizeof(sattr);
  sattr.lpSecurityDescriptor = NULL;
  sattr.bInheritHandle = TRUE;
  if (!CreatePipe(&mDesc[0], &mDesc[1], &sattr, 0)) {
#endif
    mDesc[0] = INVALID_PIPE;
    mDesc[1] = INVALID_PIPE;
  }
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
    mDesc[1] = INVALID_PIPE;
  }
}

int gcore::Pipe::read(char *buffer, int size) const {
  if (canRead()) {
#ifndef _WIN32
    int bytesRead = ::read(mDesc[0], buffer, size);
    while (bytesRead == -1 && errno == EAGAIN) {
      bytesRead = ::read(mDesc[0], buffer, size);
    }
    return bytesRead;
#else
    DWORD bytesRead = 0;
    BOOL rv = ReadFile(mDesc[0], buffer, size, &bytesRead, NULL);
    while (rv == FALSE) {
      DWORD lastErr = GetLastError();
      if (lastErr == ERROR_IO_PENDING) {
        rv = ReadFile(mDesc[0], buffer, size, &bytesRead, NULL);
      } else {
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

int gcore::Pipe::read(String &str) const {
  char rdbuf[256];

  if (canRead()) {
#ifndef _WIN32
    int bytesRead = ::read(mDesc[0], rdbuf, 255);
    while (bytesRead == -1 && errno == EAGAIN) {
      bytesRead = ::read(mDesc[0], rdbuf, 255);
    }
    if (bytesRead >= 0) {
      rdbuf[bytesRead] = '\0';
      str = rdbuf;
    }
    return bytesRead;
#else
    DWORD bytesRead = 0;
    BOOL rv = ReadFile(mDesc[0], rdbuf, 255, &bytesRead, NULL);
    while (rv == FALSE) {
      DWORD lastErr = GetLastError();
      if (lastErr == ERROR_IO_PENDING) {
        rv = ReadFile(mDesc[0], rdbuf, 255, &bytesRead, NULL);
      } else {
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
       DWORD bytesToWrite = (DWORD)size;
       DWORD bytesWritten = 0;
       BOOL rv = WriteFile(mDesc[1], buffer, bytesToWrite, &bytesWritten, NULL);
       if (rv == FALSE && GetLastError() == ERROR_IO_PENDING) {
         rv = WriteFile(mDesc[1], buffer, bytesToWrite, &bytesWritten, NULL);
       }
       return (rv == TRUE ? bytesWritten : -1);
   #endif
     }
     return -1;
}

int gcore::Pipe::write(const String &str) const {
  return write(str.c_str(), int(str.length()));
}

