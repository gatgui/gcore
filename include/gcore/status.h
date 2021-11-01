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

#ifndef __gcore_status_h__
#define __gcore_status_h__

#include <gcore/config.h>
#include <gcore/string.h>

namespace gcore
{
   class GCORE_API Status
   {
   public:
      Status();
      Status(bool success);
      Status(bool success, int errcode);
      Status(bool success, const char *msg, ...);
      Status(bool success, int errcode, const char *msg, ...);
      ~Status();
      
      Status& operator=(const Status &rhs);
      inline operator bool () const { return mSuccess; }
      inline bool operator ! () const { return !mSuccess; }
      
      void clear();
      void set(bool success);
      void set(bool success, int errcode);
      void set(bool success, const char *msg, ...);
      void set(bool success, int errcode, const char *msg, ...);
      
      inline bool succeeded() const { return mSuccess; }
      inline bool failed() const { return !mSuccess; }
      inline int errcode() const { return mErrCode; }
      inline const char* message() const { return mMsg.c_str(); }
      
   private:
      
      void set(bool success, const char *msg, va_list args);
      void set(bool success, int errcode, const char *msg, va_list args);
      
      bool mSuccess;
      int mErrCode;
      String mMsg;
   };
}

inline std::ostream& operator<<(std::ostream &os, const gcore::Status &st)
{
   os << st.message();
   return os;
}

#endif
