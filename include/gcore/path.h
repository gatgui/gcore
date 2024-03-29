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
  
  inline Path operator+(const Path &p0, const Path &p1) {
    gcore::Path rv(p0);
    rv += p1;
    return rv;
  }

  inline std::ostream& operator<<(std::ostream &os, const Path &p) {
    os << p.fullname();
    return os;
  }
  
  typedef List<Path> PathList;
  
}

#endif

