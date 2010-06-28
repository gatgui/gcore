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

#ifndef __gcore_env_h__
#define __gcore_env_h__

#include <gcore/config.h>
#include <gcore/callbacks.h>

namespace gcore {
  
  // should return false to stop iteration
  typedef Callback1wR<bool, const std::string&> EnumEnvCallback;
  GCORE_API void ForEachInEnv(const std::string &k, EnumEnvCallback cb);
  
  class EnvList : public std::vector<std::string> {
    public:
      inline EnvList() {
      }
      inline EnvList(const EnvList &rhs)
        : std::vector<std::string>(rhs) {
      }
      inline ~EnvList() {
      }
      inline EnvList& operator=(const EnvList &rhs) {
        std::vector<std::string>::operator=(rhs);
        return *this;
      }
      inline bool enumerate(const std::string &s) {
        push_back(s);
        return true;
      }
  };
  
  inline size_t ForEachInEnv(const std::string &k, EnvList &l) {
    EnumEnvCallback cb;
    MakeCallback(&l, METHOD(EnvList, enumerate), cb);
    ForEachInEnv(k, cb);
    return l.size();
  }
  
  GCORE_API std::string GetUser();
  GCORE_API std::string GetHost();
  
  GCORE_API bool HasEnv(const std::string &k);
  GCORE_API std::string GetEnv(const std::string &k);
  GCORE_API void SetEnv(const std::string &k, const std::string &v, bool overwrite);
  
  typedef std::map<std::string, std::string> EnvDict;
  GCORE_API size_t GetEnv(EnvDict &d);
  
}

#endif
