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
