/*
MIT License

Copyright (c) 2016 Gaetan Guidet

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

#include <gcore/status.h>
#include <gcore/platform.h>
#include <gcore/unicode.h>

namespace gcore
{

Status::Status()
{
   set(true);
}

Status::Status(bool success)
{
   set(success);
}

Status::Status(bool success, int errcode)
{
   set(success, errcode);
}

Status::Status(bool success, const char *msg, ...)
{
   va_list args;
   va_start(args, msg);
   set(success, msg, args);
   va_end(args);
}

Status::Status(bool success, int errcode, const char *msg, ...)
{
   va_list args;
   va_start(args, msg);
   set(success, errcode, msg, args);
   va_end(args);
}

Status::~Status()
{
}

Status& Status::operator=(const Status &rhs)
{
   mSuccess = rhs.mSuccess;
   mErrCode = rhs.mErrCode;
   mMsg = rhs.mMsg;
   return *this;
}

void Status::clear()
{
   set(true);
}

void Status::set(bool success)
{
   mSuccess = success;
   mErrCode = -1;
   mMsg = "";
}

void Status::set(bool success, int errcode)
{
   set(success);
   mErrCode = errcode;
   if (!mSuccess)
   {
      mMsg += " (";
#ifdef _WIN32
      LPWSTR buffer = NULL;
      FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
                     NULL, mErrCode, MAKELANGID(LANG_ENGLISH,SUBLANG_DEFAULT),
                     (LPWSTR)&buffer, 0, NULL);
      if (buffer)
      {
         String tmp;
         if (EncodeUTF8(buffer, tmp))
         {
            mMsg += tmp;
         }
         LocalFree(buffer);
      }
#else
      mMsg += strerror(mErrCode);
#endif
      mMsg += ")";
   }
}

void Status::set(bool success, const char *msg, ...)
{
   va_list args;
   va_start(args, msg);
   set(success, msg, args);
   va_end(args);
}

void Status::set(bool success, int errcode, const char *msg, ...)
{
   va_list args;
   va_start(args, msg);
   set(success, errcode, msg, args);
   va_end(args);
}

void Status::set(bool success, const char *msg, va_list args)
{
   set(success);
   if (msg)
   {
      char buffer[4096];
      vsnprintf(buffer, 4095, msg, args);
      mMsg = buffer;
   }
}

void Status::set(bool success, int errcode, const char *msg, va_list args)
{
   set(success, errcode);
   if (msg)
   {
      char buffer[4096];
      vsnprintf(buffer, 4095, msg, args);
      mMsg.insert(0, buffer);
   }
}

} // gcore
