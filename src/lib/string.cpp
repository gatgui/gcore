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

#include <gcore/string.h>
#include <gcore/platform.h>
#include <gcore/regexp.h>

namespace gcore {

StringList::StringList()
  : List<String>() {
}

StringList::StringList(size_t n, const String &s)
  : List<String>(n, s) {
}

StringList::StringList(const std::vector<String> &rhs)
  : List<String>(rhs) {
}

StringList::~StringList() {
}

StringList& StringList::operator=(const std::vector<String> &rhs) {
  List<String>::operator=(rhs);
  return *this;
}

StringList& StringList::filter(const Regexp &re) {
  List<String>::iterator it = begin();
  while (it != end()) {
    if (re.match(*it)) {
      ++it;
    } else {
      it = erase(it);
    }
  }
  return *this;
}

// ---

String::String() {
}

String::String(const std::string &rhs)
  : std::string(rhs) {
}

String::String(const std::string &str, size_t pos, size_t n)
  : std::string(str, pos, n) {
}

String::String(const char *s)
  : std::string(s) {
}

String::String(const char *s, size_t n)
  : std::string(s, n) {
}

String::String(char c)
  : std::string() {
  operator=(c);
}

String::String(unsigned char uc)
  : std::string() {
  operator=(uc);
}

String::String(short s)
  : std::string() {
  operator=(s);
}

String::String(unsigned short us)
  : std::string() {
  operator=(us);
}

String::String(int i)
  : std::string() {
  operator=(i);
}

String::String(unsigned int ui)
  : std::string() {
  operator=(ui);
}

String::String(long l)
  : std::string() {
  operator=(l);
}

String::String(unsigned long ul)
  : std::string() {
  operator=(ul);
}

String::String(float f)
  : std::string() {
  operator=(f);
}

String::String(double d)
  : std::string() {
  operator=(d);
}

String::String(bool b)
  : std::string() {
  operator=(b);
}

String::~String() {
}

String& String::operator=(const std::string &rhs) {
  std::string::operator=(rhs);
  return *this;
}

String& String::operator=(const char *s) {
  std::string::operator=(s);
  return *this;
}

String& String::operator=(char c) {
  std::string::operator=(c);
  return *this;
}

String& String::operator=(unsigned char uc) {
  char buffer[64];
  sprintf(buffer, "%hhu", uc);
  *this = buffer;
  return *this;
}

String& String::operator=(short s) {
  char buffer[64];
  sprintf(buffer, "%hd", s);
  *this = buffer;
  return *this;
}

String& String::operator=(unsigned short us) {
  char buffer[64];
  sprintf(buffer, "%hu", us);
  *this = buffer;
  return *this;
}

String& String::operator=(int i) {
  char buffer[64];
  sprintf(buffer, "%d", i);
  *this = buffer;
  return *this;
}

String& String::operator=(unsigned int ui) {
  char buffer[64];
  sprintf(buffer, "%u", ui);
  *this = buffer;
  return *this;
}

String& String::operator=(long l) {
  char buffer[64];
  sprintf(buffer, "%ld", l);
  *this = buffer;
  return *this;
}

String& String::operator=(unsigned long ul) {
  char buffer[64];
  sprintf(buffer, "%lu", ul);
  *this = buffer;
  return *this;
}

String& String::operator=(float f)
{
  char buffer[64];
  sprintf(buffer, "%f", f);
  *this = buffer;
  return *this;
}

String& String::operator=(double d) {
  char buffer[64];
  sprintf(buffer, "%lf", d);
  *this = buffer;
  return *this;
}

String& String::operator=(bool b) {
  *this = (b ? "true" : "false");
  return *this;
}

String& String::strip() {
  size_t p = find_first_not_of(" \t\r\n\v");
  if (p == npos) {
    assign("");
  } else {
    erase(0, p);
    p = find_last_not_of(" \t\r\n\v");
    if (p != npos) {
      erase(p+1);
    }
  }
  return *this;
}

size_t String::split(char c, StringList &l) const {//List &l) const {
  l.clear();
  size_t p0 = 0;
  size_t p1 = find(c, p0);
  while (p1 != npos) {
    l.push_back(substr(p0, p1-p0));
    p0 = p1 + 1;
    p1 = find(c, p0);
  }
  l.push_back(substr(p0));
  return l.size();
}

String& String::tolower()
{
  for (size_t i=0; i<length(); ++i) {
    if ((*this)[i] >= 'A' && (*this)[i] <= 'Z') {
      (*this)[i] = 'a' + ((*this)[i] - 'A');
    }
  }
  return *this;
}

String& String::toupper() {
  for (size_t i=0; i<length(); ++i) {
    if ((*this)[i] >= 'a' && (*this)[i] <= 'z') {
      (*this)[i] = 'A' + ((*this)[i] - 'a');
    }
  }
  return *this;
}

String& String::replace(char chr, char by, int maxCount) {
  if (maxCount == 0) {
    return *this;
  }
  
  int count = 0;
  
  size_t p0 = 0;
  size_t p1 = find(chr, p0);
  
  while (p1 != npos) {
    (*this)[p1] = by;
    if (++count == maxCount) {
      break;
    }
    p0 = p1 + 1;
    p1 = find(chr, p0);
  }
  
  return *this;
}

String& String::replace(const std::string &str, const std::string &by, int maxCount) {
  if (maxCount == 0) {
    return *this;
  }
  
  int count = 0;
  
  size_t p0 = 0;
  size_t p1 = find(str, p0);
  
  while (p1 != npos) {
    std::string::replace(begin()+p1, begin()+p1+str.length(), by.begin(), by.end());
    if (++count == maxCount) {
      break;
    }
    p0 = p1 + by.length();
    p1 = find(str, p0);
  }
  
  return *this;
}

String& String::replace(const char *str, const char *by, int maxCount) {
  if (maxCount == 0) {
    return *this;
  }
  
  int count = 0;
  
  size_t slen = strlen(str);
  size_t blen = strlen(by);
  size_t p0 = 0;
  size_t p1 = find(str, p0);
  
  while (p1 != npos) {
    std::string::replace(begin()+p1, begin()+p1+slen, by, by+blen);
    if (++count == maxCount) {
      break;
    }
    p0 = p1 + blen;
    p1 = find(str, p0);
  }
  
  return *this;
}

bool String::startswith(const std::string &st) const {
  if (st.length() > length()) {
    return false;
  }
  return (compare(0, st.length(), st) == 0);
}

bool String::startswith(const char *st) const {
  size_t stlen = strlen(st);
  if (stlen > length()) {
    return false;
  }
  return (compare(0, stlen, st) == 0);
}

String String::join(const StringList &lst) const {//List &lst) const {
  String rv;
  if (lst.size() > 0) {
    rv.append(lst[0]);
    for (size_t i=1; i<lst.size(); ++i) {
      rv.append(*this + lst[i]);
    }
  }
  return rv;
}

size_t String::count(char c) const {
  size_t n = 0;
  size_t p0 = 0;
  size_t p1 = find(c, p0);
  while (p1 != npos) {
    ++n;
    p0 = p1 + 1;
    p1 = find(c, p0);
  }
  return n;
}

int String::casecompare(const String &s) const {
#ifdef _WIN32
#	if _MSC_VER >= 1400
  return _stricmp(c_str(), s.c_str());
#	else
  return stricmp(c_str(), s.c_str());
#	endif
#else
  return strcasecmp(c_str(), s.c_str());
#endif
}

int String::casecompare(const char *s) const {
#ifdef _WIN32
#	if _MSC_VER >= 1400
  return _stricmp(c_str(), s);
#	else
  return stricmp(c_str(), s);
#	endif
#else
  return strcasecmp(c_str(), s);
#endif
}

bool String::match(const std::string &ex, RegexpMatch *m) const {
  Regexp re(ex.c_str(), REX_CAPTURE);
  RegexpMatch md;
  if (!m) {
    m = &md;
  }
  return (re.match(*this, *m));
}

bool String::match(const char *ex, RegexpMatch *m) const {
  Regexp re(ex, REX_CAPTURE);
  RegexpMatch md;
  if (!m) {
    m = &md;
  }
  return (re.match(*this, *m));
}

String& String::subst(const std::string &ex, const std::string &by) {
  Regexp re(ex.c_str(), REX_CAPTURE);
  RegexpMatch md;
  if (re.match(*this, md)) {
    assign(re.substitute(md, by));
  }
  return *this;
}

String& String::subst(const char *ex, const char *by) {
  Regexp re(ex, REX_CAPTURE);
  RegexpMatch md;
  if (re.match(*this, md)) {
    assign(re.substitute(md, by));
  }
  return *this;
}

bool String::toChar(char &c) const {
  return (sscanf(c_str(), "%hhd", &c) == 1);
}

bool String::toUChar(unsigned char &uc) const {
  return (sscanf(c_str(), "%hhu", &uc) == 1);
}

bool String::toShort(short &s) const {
  return (sscanf(c_str(), "%hd", &s) == 1);
}

bool String::toUShort(unsigned short &us) const {
  return (sscanf(c_str(), "%hu", &us) == 1);
}

bool String::toInt(int &i) const {
  return (sscanf(c_str(), "%d", &i) == 1);
}

bool String::toUInt(unsigned int &ui) const {
  return (sscanf(c_str(), "%u", &ui) == 1);
}

bool String::toLong(long &l) const {
  return (sscanf(c_str(), "%ld", &l) == 1);
}

bool String::toULong(unsigned long &ul) const {
  return (sscanf(c_str(), "%lu", &ul) == 1);
}

bool String::toFloat(float &f) const {
  return (sscanf(c_str(), "%f", &f) == 1);
}

bool String::toDouble(double &d) const {
  return (sscanf(c_str(), "%lf", &d) == 1);
}

bool String::toBool(bool &b) const {
  if (compare("0") == 0 || casecompare("false") == 0 || casecompare("off") == 0) {
    b = false;
    return true;
  } else if (compare("1") == 0 || casecompare("true") == 0 || casecompare("on") == 0) {
    b = true;
    return true;
  }
  return false;
}

}

