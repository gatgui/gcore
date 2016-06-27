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

#include <gcore/functor.h>
#include <gcore/string.h>
#include <gcore/list.h>
#include <gcore/path.h>

namespace gcore {
  
  class GCORE_API Env {
    public:
      
      typedef Functor1wR<bool, const Path&> EachInPathFunc;
      
      static String GetUser();
      static String GetHost();
      static String Get(const String &k);
      static void Set(const String &k, const String &v, bool overwrite);
      static void SetAll(const StringDict &d, bool overwrite);
      static bool IsSet(const String &k);
      static void EachInPath(const String &e, EachInPathFunc callback);
      static size_t ListPaths(const String &e, PathList &l);
      
    public:
      
      Env();
      Env(bool verbose);
      ~Env();
      
      void push();
      void pop();
      
      bool isSet(const String &key) const;
      String get(const String &key) const;
      void set(const String &key, const String &val, bool overwrite);
      void setAll(const StringDict &d, bool overwrite);
      
      size_t asDict(StringDict &d) const;
      
    protected:
      
      List<StringDict> mEnvStack;
      bool mVerbose;
  };
}

#endif
