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

namespace gcore {
  
  class GCORE_API Path {
    public:
      
      static Path GetCurrentDir();
      
    public:
      
      typedef Functor1wR<bool, const Path &> EachFunc;
      
      enum EachTarget
      {
        ET_FILE      = 0x01,
        ET_DIRECTORY = 0x02,
        ET_HIDDEN    = 0x04, // file/dir starting with a .
        ET_ALL       = ET_FILE|ET_DIRECTORY|ET_HIDDEN
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
      inline bool operator!=(const Path &rhs) const {
        return !operator==(rhs);
      }
      
      // those will use DIR_SEP
      operator const String& () const;
      operator String& ();
      
      // can use negative numbers -> index from the end
      String& operator[](int idx);
      const String& operator[](int idx) const;
      
      bool isAbsolute() const;
      
      // if path is relative, prepend current directory
      // but keeps . and ..
      Path& makeAbsolute();
      
      // remove any . or .. and make absolute if necessary
      Path& normalize();
      
      String basename() const;
      String dirname(char sep=DIR_SEP) const;
      String fullname(char sep=DIR_SEP) const;
      
      bool isDir() const;
      bool isFile() const;
      
      bool exists() const;
      
      Date lastModification() const;
      
      // file extension without .
      String getExtension() const;
      bool checkExtension(const String &ext) const;
      size_t fileSize() const;
      
      bool createDir(bool recursive=false) const;
      bool removeFile() const;
      
      void each(EachFunc cb, bool recurse=false, unsigned short flags=ET_ALL) const;
      size_t listDir(List<Path> &l, bool recurse=false, unsigned short flags=ET_ALL) const;
      
      String pop();
      Path& push(const String &s);
      
    protected:
      
      StringList mPaths;
      String mFullName;
  };
  
  typedef List<Path> PathList;
  
}

inline gcore::Path operator+(const gcore::Path &p0, const gcore::Path &p1) {
  gcore::Path rv(p0);
  rv += p1;
  return rv;
}

inline std::ostream& operator<<(std::ostream &os, const gcore::Path &p) {
  os << p.fullname();
  return os;
}

#endif

