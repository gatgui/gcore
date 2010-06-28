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

size_t Split(const std::string &str, char c, std::vector<std::string> &splits) {
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

}
}

