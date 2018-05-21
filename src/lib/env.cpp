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

#include <gcore/env.h>
#include <gcore/platform.h>
#include <gcore/log.h>

namespace gcore
{

namespace details
{
   class PathLister
   {
   public:
      PathLister(PathList &l)
         : mLst(l)
      {
      }
      bool pathItem(const Path &item)
      {
         mLst.push(item);
         return true;
      }
   private:
      PathLister();
      PathLister& operator=(const PathLister &);
   protected:
      PathList &mLst;
   };
} // details

// ---

String Env::Username()
{
#ifdef _WIN32
   // other ?
   return Get("USERNAME");
#else
   return Get("USER");
#endif
}

String Env::Hostname()
{
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

String Env::Get(const String &k)
{
   return Env().get(k);
}

void Env::Set(const String &k, const String &v, bool ow)
{
   Env().set(k, v, ow);
}

void Env::Set(const StringDict &d, bool ow)
{
   Env().set(d, ow);
}

void Env::Unset(const String &k)
{
   Env().unset(k);
}

bool Env::IsSet(const String &k)
{
   return Env().isSet(k);
}

void Env::ForEachInPath(const String &e, Env::ForEachInPathFunc callback)
{
   if (e.length() == 0 || callback == 0)
   {
      return;
   }
   char *envVal = getenv(e.c_str());
   if (envVal)
   {
      String v = envVal;
      StringList lst;
      v.split(PATH_SEP, lst);
      for (size_t i=0; i<lst.size(); ++i)
      {
         if (lst[i].length() > 0)
         {
            gcore::Path path(lst[i]);
            if (!callback(path))
            {
               return;
            }
         }
      }
   }
}

size_t Env::ListPaths(const String &e, PathList &l)
{
   ForEachInPathFunc func;
   details::PathLister pl(l);
   Bind(&pl, &details::PathLister::pathItem, func);
   l.clear();
   ForEachInPath(e, func);
   return l.size();
}

// ---

Env::Env()
   : mVerbose(false)
{
}

Env::Env(bool verbose)
   : mVerbose(verbose)
{
}

Env::~Env()
{
   mEnvStack.clear();
}

void Env::push()
{
   StringDict d;
   mEnvStack.push(d);
   asDict(mEnvStack.back());
}

void Env::pop()
{
   if (mEnvStack.size() > 0)
   {
      // get current environment
      StringDict cur;
      asDict(cur);
      
      // get last pushed environment and restore it
      StringDict &last = mEnvStack.back();
      set(last, true);
      
      // reset any keys that where not in pushed environment
      StringDict::iterator it = cur.begin();
      while (it != cur.end())
      {
         if (last.find(it->first) == last.end())
         {
            set(it->first, "", true);
         }
         ++it;
      }
      
      mEnvStack.pop();
   }
}

void Env::set(const StringDict &d, bool overwrite)
{
   StringDict::const_iterator it = d.begin();
   while (it != d.end())
   {
      set(it->first, it->second, overwrite);
      ++it;
   }
}

bool Env::isSet(const String &k) const
{
   return (get(k) != "");
}

String Env::get(const String &k) const
{
   String rv;
#ifndef _WIN32
   char *v = getenv(k.c_str());
   if (v != NULL)
   {
      rv = v;
   }
#else
   DWORD sz = GetEnvironmentVariableA(k.c_str(), NULL, 0);
   if (sz > 0)
   {
      rv.resize(sz-1);
      GetEnvironmentVariableA(k.c_str(), (char*) rv.data(), sz);
   }
#endif
   return rv;
}

void Env::set(const String &k, const String &v, bool overwrite)
{
   if (v.length() == 0)
   {
#ifndef _WIN32
      if (overwrite && getenv(k.c_str()) != NULL)
#else
      if (overwrite && GetEnvironmentVariableA(k.c_str(), NULL, 0) != 0)
#endif
      {
         unset(k);
      }
      return;
   }
   else
   {
#ifndef _WIN32
      setenv(k.c_str(), v.c_str(), (overwrite ? 1 : 0));
#else
      if (overwrite || GetEnvironmentVariableA(k.c_str(), NULL, 0) == 0)
      {
         SetEnvironmentVariableA(k.c_str(), v.c_str());
      }
#endif
   }
}

void Env::unset(const String &k)
{
#ifndef _WIN32  
   unsetenv(k.c_str());
#else
   SetEnvironmentVariableA(k.c_str(), NULL);
#endif
}

size_t Env::asDict(StringDict &d) const
{
   d.clear();
#ifndef _WIN32
   int idx = 0;
   char *curvar = environ[idx];
   while (curvar != 0)
   {
      char *es = strchr(curvar, '=');
      if (es != 0)
      {
         size_t klen = es - curvar;
         size_t vlen = strlen(curvar) - klen - 1;
         if (klen > 0)
         {
            String key, val;
            key.insert(0, curvar, klen);
            val.insert(0, es+1, vlen);
            d[key] = val;
         }
         else
         {
            if (mVerbose)
            {
               Log::PrintInfo("[gcore] Env::asDict: Ignore env string \"%s\"", curvar);
            }
         }
      }
      ++idx;
      curvar = environ[idx];
   }
#else
   char *curenv = GetEnvironmentStringsA();
   char *curvar = curenv;
   while (*curvar != '\0')
   {
      size_t len = strlen(curvar);
      char *es = strchr(curvar, '=');
      if (es != 0)
      {
         if (es == curvar)
         { 
            if (mVerbose)
            {
               Log::PrintInfo("[gcore] Env::asDict: Ignore env string \"%s\"", curvar);
            }
         }
         else
         {
            *es = '\0';
            if (strlen(curvar) > 0)
            {
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

} // gcore
