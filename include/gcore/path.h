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

#ifndef __gcore_path_h_
#define __gcore_path_h_

#include <gcore/functor.h>
#include <gcore/string.h>
#include <gcore/list.h>
#include <gcore/platform.h>
#include <gcore/date.h>

namespace gcore
{
   class GCORE_API Path
   {
   public:
      
      static Path CurrentDir();
      
   public:
      
      typedef Functor1wR<bool, const Path &> ForEachFunc;
      
      enum ForEachTarget
      {
         FE_FILE      = 0x01,
         FE_DIRECTORY = 0x02,
         FE_HIDDEN    = 0x04, // file/dir starting with a .
         FE_ALL       = FE_FILE|FE_DIRECTORY|FE_HIDDEN
      };
      
   public:
      
      Path();
      Path(const char *s);
      Path(const String &s);
      Path(const Path &rhs);
      ~Path();
      
      Path& operator=(const Path &rhs);
      Path& operator=(const String &s);
      Path& operator=(const char *s);
      
      Path& operator+=(const Path &rhs);
      
      bool operator==(const Path &rhs) const;
      bool operator!=(const Path &rhs) const;
      
      // those will use '/'
      operator const String& () const;
      operator String& ();
      
      // can use negative numbers -> index from the end
      int depth() const;
      String& operator[](int idx);
      const String& operator[](int idx) const;
      
      bool isAbsolute() const;
      
      // if path is relative, prepend current directory
      // but keeps . and ..
      Path& makeAbsolute();
      
      // remove any . or .. and make absolute if necessary
      Path& normalize();
      
      String basename() const;
      String dirname(char sep='/') const;
      String fullname(char sep='/') const;
      
      bool isDir() const;
      bool isFile() const;
      
      bool exists() const;
      
      Date lastModification() const;
      
      // file extension without .
      String extension() const;
      bool checkExtension(const String &ext) const;
      size_t fileSize() const;
      
      bool createDir(bool recursive=false) const;
      bool removeFile() const;
      
      // flags is a bit wise combination of constants defined in ForEachTarget enum
      void forEach(ForEachFunc cb, bool recurse=false, unsigned short flags=FE_ALL) const;
      size_t listDir(List<Path> &l, bool recurse=false, unsigned short flags=FE_ALL) const;
      
      String pop();
      Path& push(const String &s);
      
   protected:
      
      StringList mPaths;
      String mFullName;
   };
   
   inline bool Path::operator!=(const Path &rhs) const
   {
      return !operator==(rhs);
   }
   
   inline int Path::depth() const
   {
      return int(mPaths.size());
   }
   
   inline Path operator+(const Path &p0, const Path &p1)
   {
      Path rv(p0);
      rv += p1;
      return rv;
   }

   inline std::ostream& operator<<(std::ostream &os, const Path &p)
   {
      os << p.fullname();
      return os;
   }
   
   typedef List<Path> PathList;
   
}

#endif

