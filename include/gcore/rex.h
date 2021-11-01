/*
MIT License

Copyright (c) 2010 Gaetan Guidet

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

#ifndef __gcore_rex_h_
#define __gcore_rex_h_

#include <gcore/string.h>
#include <gcore/list.h>

// ignore escape character in string
#define RAW(str) (gcore::_RawString(#str))

namespace gcore {

   struct GCORE_API _RawString
   {
      _RawString(const char *s);
      operator String () const;
      
   private:
      String e;
   };

   class GCORE_API RexMatch
   {
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
      String group(const String &n) const;
   
      // offset in matched string (group(0))
      size_t offset(size_t i) const;

      size_t length(size_t i) const;
   
      size_t groupCount() const;
   
      bool hasGroup(size_t i) const;
      bool hasNamedGroup(const String &n) const;
      
      void clear();
   
   protected:
   
      String mStr;
      Range mRange;
      List<Range> mGroups; // 0 if full match
      std::map<String, size_t> mNamedGroups;
   };

   class GCORE_API Rex
   {
   public:
   
      // Remove Consume, Capture, Not and Inherit stuffs
      enum Flags
      {
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
      
      bool search(const String &s, RexMatch &m, unsigned short flags=0) const;
      bool search(const String &s, size_t offset, RexMatch &m, unsigned short flags=0) const;
      bool search(const String &s, size_t offset, size_t len, RexMatch &m, unsigned short flags=0) const;
      
      bool search(const String &s, unsigned short flags=0) const;
      bool search(const String &s, size_t offset, unsigned short flags=0) const;
      bool search(const String &s, size_t offset, size_t len, unsigned short flags=0) const;
   
      bool match(const String &s, RexMatch &m, unsigned short flags=0) const;
      bool match(const String &s, size_t offset, RexMatch &m, unsigned short flags=0) const;
      bool match(const String &s, size_t offset, size_t len, RexMatch &m, unsigned short flags=0) const;
      
      bool match(const String &s, unsigned short flags=0) const;
      bool match(const String &s, size_t offset, unsigned short flags=0) const;
      bool match(const String &s, size_t offset, size_t len, unsigned short flags=0) const;
      
      String substitute(const RexMatch &m, const String &in) const;
      String substitute(const String &in, const String &by, int maxCount=-1) const;
   
      friend GCORE_API std::ostream& operator<<(std::ostream &os, const Rex &r);
   
   protected:
   
      bool mValid;
      String mExp;
      class Instruction *mCode;
      int mNumGroups;
   };

}

#endif


