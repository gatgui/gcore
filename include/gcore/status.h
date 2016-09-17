/*

Copyright (C) 2016~  Gaetan Guidet

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
