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

#include <gcore/env.h>
#include <gcore/platform.h>

/*
#ifdef _WIN32
//# include <winsock2.h> // gethostname, requires linking winsock32
# define _WIN32_WINNT 0x0500
# include <windows.h>
#endif
*/

#ifndef _WIN32
# ifdef __APPLE__
#  include <crt_externs.h>
#  define environ (*_NSGetEnviron())
# else
extern char **environ;
# endif
#endif

namespace gcore {

std::string GetUser() {
#ifdef _WIN32
  // other ?
  return GetEnv("USERNAME");
#else
  return GetEnv("USER");
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

void ForEachInEnv(const std::string &e, EnumEnvCallback callback) {
  if (e.length() == 0 || callback == 0) {
    return;
  }
  char *envVal = getenv(e.c_str());
  if (envVal) {
    std::string v = envVal;
    //std::cout << e << " = \"" << v << "\"" << std::endl;
    size_t p0 = 0, p1 = v.find(PATH_SEP, p0);
    while (p1 != std::string::npos) {
      std::string tmp = v.substr(p0, p1 - p0);
      if (tmp.length() > 0) {
        if (!callback(tmp)) {
          return;
        }
      }
      p0 = p1 + 1;
      p1 = v.find(PATH_SEP, p0);
    }
    // and last one
    if (p0 < v.length()) {
      std::string tmp = v.substr(p0);
      if (tmp.length() > 0) {
        callback(tmp);
      }
    }
  }
}

bool HasEnv(const std::string &k) {
#ifndef _WIN32
  return (getenv(k.c_str()) != 0);
#else
  return (GetEnvironmentVariableA(k.c_str(), NULL, 0) > 0);
#endif
}

std::string GetEnv(const std::string &k) {
  std::string rv;
#ifndef _WIN32
  char *v = getenv(k.c_str());
  if (v != NULL) {
    rv = v;
  }
#else
  DWORD sz = GetEnvironmentVariableA(k.c_str(), NULL, 0);
  if (sz > 0) {
    rv.resize(sz-1);
    GetEnvironmentVariableA(k.c_str(), (char*) rv.data(), sz);
  }
#endif
  return rv;
}

void SetEnv(const std::string &k, const std::string &v, bool overwrite) {
#ifndef _WIN32  
  setenv(k.c_str(), v.c_str(), (overwrite ? 1 : 0));
#else
  std::string dummy;
  if (overwrite || GetEnvironmentVariableA(k.c_str(), NULL, 0) == 0) {
    SetEnvironmentVariableA(k.c_str(), v.c_str());
  }
#endif
}

size_t GetEnv(EnvDict &d) {
  d.clear();
#ifndef _WIN32
  int idx = 0;
  char *curvar = environ[idx];
  while (curvar != 0) {
    char *es = strchr(curvar, '=');
    if (es != 0) {
      size_t klen = es - curvar;
      size_t vlen = strlen(curvar) - klen - 1;
      if (klen > 0) {
        std::string key, val;
        key.insert(0, curvar, klen);
        val.insert(0, es+1, vlen);
        d[key] = val;
      } else {
        //std::cerr << "Ignore env string \"" << curvar << "\"" << std::endl;
      }
    }
    ++idx;
    curvar = environ[idx];
  }
#else
  char *curenv = GetEnvironmentStringsA();
  char *curvar = curenv;
  while (*curvar != '\0') {
    size_t len = strlen(curvar);
    char *es = strchr(curvar, '=');
    if (es != 0) {
      if (es == curvar) { 
        //std::cerr << "Ignore env string \"" << curvar << "\"" << std::endl;
      } else {
        *es = '\0';
        if (strlen(curvar) > 0) {
          d[curvar] = es+1;
        }
      }
    }
    curvar += len + 1;
  }
  FreeEnvironmentStrings(curenv);
#endif
  return d.size();
}
  
}
