/*
MIT License

Copyright (c) 2009 Gaetan Guidet

This file is part of gcore.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <gcore/pipe.h>
#include <gcore/platform.h>

namespace gcore
{

PipeID Pipe::StdInID()
{
#ifdef _WIN32
   return GetStdHandle(STD_INPUT_HANDLE);
#else
   return fileno(stdin);
#endif
}

PipeID Pipe::StdOutID()
{
#ifdef _WIN32
   return GetStdHandle(STD_OUTPUT_HANDLE);
#else
   return fileno(stdout);
#endif
}

PipeID Pipe::StdErrID()
{
#ifdef _WIN32
   return GetStdHandle(STD_ERROR_HANDLE);
#else
   return fileno(stderr);
#endif
}

Pipe::Pipe()
   : mOwn(false)
#ifdef _WIN32
   , mConnected(false)
#endif
{
   mDesc[0] = INVALID_PIPE;
   mDesc[1] = INVALID_PIPE;
}

Pipe::Pipe(PipeID rid, PipeID wid)
   : mOwn(false)
#ifdef _WIN32
   , mConnected(false)
#endif
{
   mDesc[0] = rid;
   mDesc[1] = wid;
}

Pipe::Pipe(const Pipe &rhs)
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

Pipe::~Pipe() {
   // Don't close pipes (leave responsability to client)
}

Pipe& Pipe::operator=(const Pipe &rhs)
{
   if (this != &rhs)
   {
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

PipeID Pipe::readID() const
{
   return mDesc[0];
}

PipeID Pipe::writeID() const
{
   return mDesc[1];
}

void Pipe::close()
{
   if (isNamed() && isOwned())
   {
#ifndef _WIN32
      String path = "/tmp/" + mName;
      closeRead();
      closeWrite();
      unlink(path.c_str());
#else
      if (mConnected)
      {
         FlushFileBuffers(mDesc[1]);
         DisconnectNamedPipe(mDesc[0]);
      }
      closeRead();
      closeWrite();
#endif
   }
   else
   {
      closeRead();
      closeWrite();
   }
   mName = "";
   mOwn = false;
#ifdef _WIN32
   mConnected = false;
#endif
}

Status Pipe::open(const String &name)
{
   close();
#ifndef _WIN32
   String path = "/tmp/" + name;
   int fd = ::open(path.c_str(), O_RDWR);
   if (fd != -1)
   {
      mName = name;
      mDesc[0] = fd;
      mDesc[1] = mDesc[0];
      return Status(true);
   }
#else
   String pipename = "\\\\.\\pipe\\" + name;
   HANDLE hdl = CreateFile(pipename.c_str(), GENERIC_READ | GENERIC_WRITE,
                                       0, NULL, OPEN_EXISTING, 0, NULL);
   if (hdl != INVALID_HANDLE_VALUE)
   {
      mName = name;
      mDesc[0] = hdl;
      mDesc[1] = mDesc[0];
      return Status(true);
   }
#endif
   return Status(false, std_errno(), "gcore::Pipe::open");
}

Status Pipe::create()
{
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
      return Status(false, std_errno(), "gcore::Pipe::create");
   }
   else
   {
      mOwn = true;
      return Status(true);
   }
}

Status Pipe::create(const String &name)
{
   close();
#ifndef _WIN32
   String path = "/tmp/" + name;
   if (mkfifo(path.c_str(), 0666) == 0)
   {
      int fd = ::open(path.c_str(), O_RDWR);
      if (fd != -1)
      {
         mOwn = true;
         mName = name;
         mDesc[0] = fd;
         mDesc[1] = mDesc[0];
         return Status(true);
      }
   }
#else
   // Note: May want to expose in/out buffer size
   String pipename = "\\\\.\\pipe\\" + name;
   HANDLE hdl = CreateNamedPipe(pipename.c_str(), PIPE_ACCESS_DUPLEX,
                                              PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                                              1, 4096, 4096, 0, NULL);
   if (hdl != INVALID_HANDLE_VALUE)
   {
      mOwn = true;
      mName = name;
      mConnected = false;
      mDesc[0] = hdl;
      mDesc[1] = mDesc[0];
      return Status(true);
   }
#endif
   return Status(false, std_errno(), "gcore::Pipe::create");
}

bool Pipe::isNamed() const
{
   return (mName.length() > 0);
}

const String& Pipe::name() const
{
   return mName;
}

bool Pipe::isOwned() const
{
   return mOwn;
}

bool Pipe::canRead() const
{
   return (IsValidPipeID(mDesc[0]));
}

bool Pipe::canWrite() const
{
   return (IsValidPipeID(mDesc[1]));
}

void Pipe::closeRead()
{
   if (canRead())
   {
#ifndef _WIN32
      ::close(mDesc[0]);
#else
      CloseHandle(mDesc[0]);
#endif
      if (mDesc[1] == mDesc[0])
      {
         mDesc[1] = INVALID_PIPE;
      }
      mDesc[0] = INVALID_PIPE;
   }
}

void Pipe::closeWrite()
{
   if (canWrite())
   {
#ifndef _WIN32
      ::close(mDesc[1]);
#else 
      CloseHandle(mDesc[1]);
#endif
      if (mDesc[0] == mDesc[1])
      {
         mDesc[0] = INVALID_PIPE;
      }
      mDesc[1] = INVALID_PIPE;
   }
}

int Pipe::read(char *buffer, int size, Status *status) const
{
   if (!buffer)
   {
      if (status)
      {
         status->set(false, "gcore::Pipe::read: Invalid buffer");
      }
      return 0;
   }
   else if (size <= 1)
   {
      if (status)
      {
         status->set(false, "gcore::Pipe::read: Insufficient buffer size");
      }
      if (size > 0)
      {
         buffer[0] = '\0';
      }
      return 0;
   }
   if (canRead())
   {
#ifndef _WIN32
      int bytesRead = ::read(mDesc[0], buffer, size-1);
      while (bytesRead == -1)
      {
         if (errno == EAGAIN)
         {
            // makes the call blocking
            bytesRead = ::read(mDesc[0], buffer, size-1);
         }
         else if (errno == EINTR)
         {
            // interrupted, consider as error
            break;
         }
         else
         {
            break;
         }
      }
      if (bytesRead == -1)
      {
         if (status)
         {
            status->set(false, std_errno(), "gcore::Pipe::read");
         }
         buffer[0] = '\0';
         return 0;
      }
      else
      {
         if (status)
         {
            status->set(true);
         }
         buffer[bytesRead] = '\0';
         return bytesRead;
      }
#else
      bool namedPipeServer = (isNamed() && isOwned());
      if (namedPipeServer && !mConnected)
      {
         if (ConnectNamedPipe(mDesc[0], NULL))
         {
            mConnected = true;
         }
         else
         {
            if (status)
            {
               status->set(false, std_errno(), "gcore::Pipe::read");
            }
            buffer[0] = '\0';
            return 0;
         }
      }
      DWORD bytesRead = 0;
      BOOL rv = ReadFile(mDesc[0], buffer, size-1, &bytesRead, NULL);
      while (rv == FALSE)
      {
         DWORD lastErr = GetLastError();
         if (lastErr == ERROR_IO_PENDING)
         {
            // makes the call blocking
            rv = ReadFile(mDesc[0], buffer, size-1, &bytesRead, NULL);
         }
         else if (lastErr == ERROR_MORE_DATA)
         {
            // stop here, return what we have so far
            bytesRead = size-1;
            rv = TRUE;
         }
         else
         {
            if (namedPipeServer)
            {
               DisconnectNamedPipe(mDesc[0]);
               mConnected = false;
            }
            if (lastErr == ERROR_HANDLE_EOF || lastErr == ERROR_BROKEN_PIPE)
            {
               // EOF, return 0 without failure
               rv = TRUE;
               bytesRead = 0;
            }
            break;
         }
      }
      if (rv)
      {
         if (status)
         {
            status->set(true);
         }
         buffer[bytesRead] = '\0';
         return bytesRead;
      }
      else
      {
         if (status)
         {
            status->set(false, std_errno(), "gcore::Pipe::read");
         }
         buffer[0] = '\0';
         return 0;
      }
#endif
   }
   if (status)
   {
      status->set(false, "gcore::Pipe::read: Pipe is closed for reading.");
   }
   buffer[0] = '\0';
   return 0;
}

int Pipe::write(const char *buffer, int size, Status *status) const
{
   if (canWrite())
   {
#ifndef _WIN32
      int bytesToWrite = size;
      int rv = ::write(mDesc[1], buffer, bytesToWrite);
      while (rv == -1 && errno == EAGAIN)
      {
         rv = ::write(mDesc[1], buffer, bytesToWrite);
      }
      if (rv == -1)
      {
         if (status)
         {
            status->set(false, std_errno(), "gcore::Pipe::write");
         }
         return 0;
      }
      else
      {
         if (status)
         {
            status->set(true);
         }
         return rv;
      }
#else
      bool namedPipeServer = (isNamed() && isOwned());
      if (namedPipeServer && !mConnected)
      {
         if (ConnectNamedPipe(mDesc[1], NULL))
         {
            mConnected = true;
         }
         else
         {
            if (status)
            {
               status->set(false, std_errno(), "gcore::Pipe::write");
            }
            return 0;
         }
      }
      DWORD bytesToWrite = (DWORD)size;
      DWORD bytesWritten = 0;
      BOOL rv = WriteFile(mDesc[1], buffer, bytesToWrite, &bytesWritten, NULL);
      while (rv == FALSE)
      {
         DWORD lastErr = GetLastError();
         if (lastErr == ERROR_IO_PENDING)
         {
            rv = WriteFile(mDesc[1], buffer, bytesToWrite, &bytesWritten, NULL);
         }
         else
         {
            if (namedPipeServer)
            {
               DisconnectNamedPipe(mDesc[1]);
               mConnected = false;
            }
            break;
         }
      }
      if (!rv)
      {
         if (status)
         {
            status->set(false, std_errno(), "gcore::Pipe::write");
         }
         return 0;
      }
      else
      {
         if (status)
         {
            status->set(true);
         }
         return bytesWritten;
      }
#endif
   }
   if (status)
   {
      status->set(false, "gcore::Pipe::write: Pipe is closed for writing.");
   }
   return 0;
}

int Pipe::write(const String &str, Status *status) const
{
   return write(str.c_str(), int(str.length()), status);
}

} // gcore
