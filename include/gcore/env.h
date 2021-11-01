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
