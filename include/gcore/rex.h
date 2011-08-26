/*

Copyright (C) 2010  Gaetan Guidet

This file is part of gcore.

rex is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at
your option) any later version.

rex is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
USA.

*/

#ifndef __gcore_rex_h_
#define __gcore_rex_h_

#include <gcore/string.h>
#include <gcore/list.h>

// ignore escape character in string
#define RAW(str) (gcore::_RawString(#str))

namespace gcore {

  struct GCORE_API _RawString {
    
    _RawString(const char *s);
    operator String () const;
    
    private:
      String e;
  };

  class GCORE_API RexMatch {
    public:
    
      friend class Rex;
    
      typedef std::pair<int,int> Range;
    
      RexMatch();
      RexMatch(const RexMatch &rhs);
      ~RexMatch();
    
      RexMatch& operator=(const RexMatch &rhs);
    
      String pre() const;
    
      String post() const;
    
      String group(size_t i) const;
      String group(const std::string &n) const;
    
      // offset in matched string (group(0))
      size_t offset(size_t i) const;
  
      size_t length(size_t i) const;
    
      size_t numGroups() const;
    
      bool hasGroup(size_t i) const;
      bool hasNamedGroup(const std::string &n) const;
    
    protected:
    
      String mStr;
      Range mRange;
      List<Range> mGroups; // 0 if full match
      std::map<std::string, size_t> mNamedGroups;
  };

  class GCORE_API Rex {
    public:
    
      // Remove Consume, Capture, Not and Inherit stuffs
      enum Flags {
        NoCase          = 0x0001, // ignore case
        Reverse         = 0x0008, // apply backwards (no group capture)
        Multiline       = 0x0010, // makes ^ and $ match at line boundaries (instead of buffer like \A \Z)
        DotMatchNewline = 0x0040, // dot matches new line chars \r and/or \n
      };
    
    public:
    
      Rex();
      Rex(const String &exp);
      Rex(const Rex &rhs);
      ~Rex();
    
      Rex& operator=(const Rex &rhs);
    
      bool valid() const;
    
      void set(const String &exp);
      const String get() const;
    
      bool search(const String &s, RexMatch &m, unsigned short flags=0, size_t offset=0, size_t len=size_t(-1)) const;
      bool search(const String &s, unsigned short execflags=0, size_t offset=0, size_t len=size_t(-1)) const;
    
      bool match(const String &s, RexMatch &m, unsigned short flags=0, size_t offset=0, size_t len=size_t(-1)) const;
      bool match(const String &s, unsigned short execflags=0, size_t offset=0, size_t len=size_t(-1)) const;
      
      String substitute(const RexMatch &m, const String &in, bool *failed=0) const;
      String substitute(const String &str, const String &in, bool *failed=0) const;
    
      friend GCORE_API std::ostream& operator<<(std::ostream &os, const Rex &r);
    
    protected:
    
      bool mValid;
      String mExp;
      class Instruction *mCode;
      int mNumGroups;
  };

}

#endif


