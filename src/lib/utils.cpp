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

#include <gcore/utils.h>
#include <gcore/platform.h>
/*
#include <cstdio>
#include <cstring>
#include <ctime>
#ifdef _WIN32
//# include <winsock2.h> // gethostname, requires linking winsock32
# define _WIN32_WINNT 0x0500
# include <windows.h>
#endif
*/

namespace gcore {

std::string GetUser() {
#ifdef _WIN32
  // other ?
  return getenv("USERNAME");
#else
  return getenv("USER");
#endif
}

std::string GetHost() {
  char buffer[1024];
#ifdef _WIN32
  DWORD sz = 1024;
  //ComputerNameDnsHostname?
  GetComputerNameEx(ComputerNamePhysicalDnsHostname, buffer, &sz);
  return buffer;
#else
  gethostname(buffer, 1024);
  buffer[1023] = '\0';
  return buffer;
#endif
}

std::string GetDate() {
  char buffer[256];
  time_t curt = time(NULL);
  struct tm *t = 0;
  t = localtime(&curt);
  sprintf(buffer, "%04d/%02d/%02d %02d:%02d:%02d", 1900+t->tm_year, 1+t->tm_mon, t->tm_mday,
                                                   t->tm_hour, t->tm_min, t->tm_sec);
  return buffer;
}

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

}

