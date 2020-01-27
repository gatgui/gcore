/*

Copyright (C) 2010~  Gaetan Guidet

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

namespace gcore
{
   class GCORE_API Env
   {
   public:
      
      class KeyCompare
      {
      public:
         inline bool operator()(const gcore::String &s1, const gcore::String &s2) const
         {
#ifdef _WIN32
            return (s1.casecompare(s2) < 0);
#else
            return s1 < s2;
#endif
         }
      };
      typedef std::map<String, String, KeyCompare> Dict;
      
      typedef Functor1wR<bool, const Path&> ForEachInPathFunc;
      
      static String Username();
      static String Hostname();
      static String Get(const String &k);
      static void Set(const String &k, const String &v, bool overwrite);
      static void Unset(const String &k);
      static void Set(const Dict &d, bool overwrite);
      static bool IsSet(const String &k);
      static void ForEachInPath(const String &e, ForEachInPathFunc callback);
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
      void set(const Dict &d, bool overwrite);
      void unset(const String &key);
      
      size_t asDict(Dict &d) const;
      
   protected:
      
      List<Dict> mEnvStack;
      bool mVerbose;
   };
}

#endif
