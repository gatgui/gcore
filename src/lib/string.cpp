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
#include <gcore/rexp.h>

namespace gcore {
namespace String {

std::string Strip(const std::string &str) {
  size_t from = str.find_first_not_of(" \t\r\n\v");
  size_t to   = str.find_last_not_of(" \t\r\n\v");
  if (from == std::string::npos) {
    return "";
  } else {
    if (to == std::string::npos) {
      return str.substr(from);
    } else {
      return str.substr(from, to-from+1);
    }
  }
}

size_t Split(const std::string &str, char c, List &splits) {
  splits.clear();
  size_t p0 = 0;
  size_t p1 = str.find(c, p0);
  while (p1 != std::string::npos) {
    splits.push_back(str.substr(p0, p1-p0));
    p0 = p1 + 1;
    p1 = str.find(c, p0);
  }
  splits.push_back(str.substr(p0));
  return splits.size();
}

std::string ToLower(const std::string &str_in) {
  std::string str = str_in;
  for (size_t i=0; i<str.length(); ++i) {
    if (str[i] >= 'A' && str[i] <= 'Z') {
      str[i] = 'a' + (str[i] - 'A');
    }
  }
  return str;
}

std::string ToUpper(const std::string &str_in) {
  std::string str = str_in;
  for (size_t i=0; i<str.length(); ++i) {
    if (str[i] >= 'a' && str[i] <= 'z') {
      str[i] = 'A' + (str[i] - 'a');
    }
  }
  return str;
}

std::string Replace(char chr, const std::string &src, char by, int maxCount) {
  std::string in = src;
  
  if (maxCount == 0) {
    return in;
  }
  
  int count = 0;
  
  size_t p0 = 0;
  size_t p1 = in.find(chr, p0);
  
  while (p1 != std::string::npos) {
    in[p1] = by;
    if (++count == maxCount) {
      break;
    }
    p0 = p1 + 1;
    p1 = in.find(chr, p0);
  }
  
  return in;
}

std::string Replace(const std::string &str, const std::string &src, const std::string &by, int maxCount) {
  std::string in = src;
  
  if (maxCount == 0) {
    return in;
  }
  
  int count = 0;
  
  size_t p0 = 0;
  size_t p1 = in.find(str, p0);
  
  while (p1 != std::string::npos) {
    in.replace(in.begin()+p1, in.begin()+p1+str.length(), by.begin(), by.end());
    if (++count == maxCount) {
      break;
    }
    p0 = p1 + by.length();
    p1 = in.find(str, p0);
  }
  
  return in;
}

bool StartsWith(const std::string &s, const std::string &st)
{
  if (st.length() > s.length())
  {
    return false;
  }
  return (s.compare(0, st.length(), st) == 0);
}

std::string Join(const std::string &joiner, const List &lst)
{
  std::string rv;
  if (lst.size() > 0)
  {
    rv.append(lst[0]);
    for (size_t i=1; i<lst.size(); ++i)
    {
      rv.append(joiner + lst[i]);
    }
  }
  return rv;
}

size_t Count(const std::string &in, char c)
{
  size_t n = 0;
  size_t p0 = 0;
  size_t p1 = in.find(c, p0);
  while (p1 != std::string::npos)
  {
    ++n;
    p0 = p1 + 1;
    p1 = in.find(c, p0);
  }
  return n;
}

bool ToChar(const std::string &s, char &out)
{
  return (sscanf(s.c_str(), "%hhd", &out) == 1);
}

bool ToUChar(const std::string &s, unsigned char &out)
{
  return (sscanf(s.c_str(), "%hhu", &out) == 1);
}

bool ToShort(const std::string &s, short &out)
{
  return (sscanf(s.c_str(), "%hd", &out) == 1);
}

bool ToUShort(const std::string &s, unsigned short &out)
{
  return (sscanf(s.c_str(), "%hu", &out) == 1);
}

bool ToInt(const std::string &s, int &out)
{
  return (sscanf(s.c_str(), "%d", &out) == 1);
}

bool ToUInt(const std::string &s, unsigned int &out)
{
  return (sscanf(s.c_str(), "%u", &out) == 1);
}

bool ToLong(const std::string &s, long &out)
{
  return (sscanf(s.c_str(), "%ld", &out) == 1);
}

bool ToULong(const std::string &s, unsigned long &out)
{
  return (sscanf(s.c_str(), "%lu", &out) == 1);
}

bool ToFloat(const std::string &s, float &out)
{
  return (sscanf(s.c_str(), "%f", &out) == 1);
}

bool ToDouble(const std::string &s, double &out)
{
  return (sscanf(s.c_str(), "%lf", &out) == 1);
}

bool ToBool(const std::string &s, bool &out)
{
  static Regexp sBoolExp(IEC("(true|1)"), REX_ICASE);
  out = sBoolExp.match(s);
  return true;
}

std::string ToString(char in)
{
  char buffer[64];
  sprintf(buffer, "%hhd", in);
  return buffer;
}

std::string ToString(unsigned char in)
{
  char buffer[64];
  sprintf(buffer, "%hhu", in);
  return buffer;
}

std::string ToString(short in)
{
  char buffer[64];
  sprintf(buffer, "%hd", in);
  return buffer;
}

std::string ToString(unsigned short in)
{
  char buffer[64];
  sprintf(buffer, "%hu", in);
  return buffer;
}

std::string ToString(int in)
{
  char buffer[64];
  sprintf(buffer, "%d", in);
  return buffer;
}

std::string ToString(unsigned int in)
{
  char buffer[64];
  sprintf(buffer, "%u", in);
  return buffer;
}

std::string ToString(long in)
{
  char buffer[64];
  sprintf(buffer, "%ld", in);
  return buffer;
}

std::string ToString(unsigned long in)
{
  char buffer[64];
  sprintf(buffer, "%lu", in);
  return buffer;
}

std::string ToString(float in)
{
  char buffer[64];
  sprintf(buffer, "%f", in);
  return buffer;
}

std::string ToString(double in)
{
  char buffer[64];
  sprintf(buffer, "%lf", in);
  return buffer;
}

std::string ToString(bool in)
{
  return (in ? "true" : "false");
}

}
}

