/*

Copyright (C) 2010  Gaetan Guidet

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

#ifndef __gcore_string_h_
#define __gcore_string_h_

#include <gcore/config.h>
#include <gcore/list.h>

namespace gcore {
  
  class GCORE_API Rex;
  class GCORE_API RexMatch;
  class GCORE_API StringList;
  
  class GCORE_API String : public std::string {
    public:
      
      //typedef std::vector<String> List;
      
      String();
      String(const std::string &rhs);
      String(const std::string &str, size_t pos, size_t n=npos);
      String(const char *s);
      String(const char *s, size_t n);
      String(char c);
      String(unsigned char uc);
      String(short s);
      String(unsigned short us);
      String(int i);
      String(unsigned int ui);
      String(long l);
      String(long long ll);
      String(unsigned long ul);
      String(unsigned long long ull);
      String(float f);
      String(double d);
      String(bool b);
      virtual ~String();
      
      String& operator=(const std::string &rhs);
      String& operator=(const char *s);
      String& operator=(char c);
      String& operator=(unsigned char uc);
      String& operator=(short s);
      String& operator=(unsigned short us);
      String& operator=(int i);
      String& operator=(unsigned int ui);
      String& operator=(long l);
      String& operator=(long long ll);
      String& operator=(unsigned long ul);
      String& operator=(unsigned long long ull);
      String& operator=(float f);
      String& operator=(double d);
      String& operator=(bool b);
      
      String& operator+=(const String &rhs);
      String& operator*=(size_t n);
      
      String operator()(long from, long to) const;
      
      String& strip();
      String& rstrip();
      String& lstrip();
      //size_t split(char c, List &l) const;
      size_t split(char c, StringList &l) const;
      String& tolower();
      String& toupper();
      String& replace(char c, char by, int maxCount=-1);
      String& replace(const std::string &s, const std::string &by, int maxCount=-1);
      String& replace(const char *s, const char *by, int maxCount=-1);
      bool startswith(const std::string &s) const;
      bool startswith(const char *s) const;
      bool endswith(const std::string &s) const;
      bool endswith(const char *s) const;
      //String join(const List &l) const;
      String join(const StringList &l) const;
      size_t count(char c) const;
      bool match(const std::string &exp, class RexMatch *m=0) const;
      bool match(const char *exp, class RexMatch *m=0) const;
      String& subst(const std::string &exp, const std::string &by, int maxCount=-1);
      String& subst(const char *exp, const char *by, int maxCount=-1);
      int casecompare(const String &s) const;
      int casecompare(const char *s) const;
      
      bool toChar(char &c) const;
      bool toUChar(unsigned char &uc) const;
      bool toShort(short &s) const;
      bool toUShort(unsigned short &us) const;
      bool toInt(int &i) const;
      bool toUInt(unsigned int &ui) const;
      bool toLong(long &l) const;
      bool toLongLong(long long &ll) const;
      bool toULong(unsigned long &ul) const;
      bool toULongLong(unsigned long long &ull) const;
      bool toFloat(float &f) const;
      bool toDouble(double &d) const;
      bool toBool(bool &b) const;
  };
  
  class GCORE_API StringList : public List<String> {
    public:
      
      StringList();
      StringList(size_t n, const String &s=String());
      StringList(const std::vector<String> &rhs);
      virtual ~StringList();
      
      StringList& operator=(const std::vector<String> &rhs);
      
      StringList& filter(const Rex &re);
      
      template <typename InputIterator>
      StringList(InputIterator first, InputIterator last)
        : List<String>(first, last) {
      }
  };
  
  typedef std::map<String, String> StringDict;
}

inline gcore::String operator*(const gcore::String &s, size_t n) {
  gcore::String rv(s);
  rv *= n;
  return rv;
}

inline gcore::String operator*(size_t n, const gcore::String &s) {
  gcore::String rv(s);
  rv *= n;
  return rv;
}

#endif

