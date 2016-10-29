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
#include <gcore/encoding.h>

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
   String rv;
#ifdef _WIN32
   std::vector<wchar_t> buffer(1024, 0);
   DWORD sz = 1024;
   // or should that be ComputerNameDnsHostname?
   if (GetComputerNameExW(ComputerNamePhysicalDnsHostname, &buffer[0], &sz))
   {
      ToMultiByteString(&buffer[0], UTF8Codepage, rv);
   }
   else
   {
      if (GetLastError() == ERROR_MORE_DATA)
      {
         buffer.resize(sz, 0);
         if (GetComputerNameExW(ComputerNamePhysicalDnsHostname, &buffer[0], &sz))
         {
            ToMultiByteString(&buffer[0], UTF8Codepage, rv);
         }
      }
   }
   
#else
   std::vector<char> buffer(1024, '\0');
   int ret = gethostname(&buffer[0], buffer.size());
   while (ret != 0 && errno == ENAMETOOLONG)
   {
      buffer.resize(2 * buffer.size(), '\0');
      ret = gethostname(&buffer[0], buffer.size());
   }
   if (ret == 0)
   {
      rv = &buffer[0];
   }
#endif
   return rv;
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
   
   String v = Env().get(e);
   if (v.length() == 0)
   {
      return;
   }
   
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
   std::wstring wk, wv;
   ToWideString(UTF8Codepage, k.c_str(), wk);
   DWORD sz = GetEnvironmentVariableW(wk.c_str(), NULL, 0);
   if (sz > 0)
   {
      wv.resize(sz - 1);
      GetEnvironmentVariableW(wk.c_str(), (wchar_t*) wv.data(), sz);
      ToMultiByteString(wv.c_str(), UTF8Codepage, rv);
   }
#endif
   return rv;
}

void Env::set(const String &k, const String &v, bool overwrite)
{
#ifndef _WIN32  
   setenv(k.c_str(), v.c_str(), (overwrite ? 1 : 0));
#else
   std::wstring wk, wv;
   ToWideString(UTF8Codepage, k.c_str(), wk);
   ToWideString(UTF8Codepage, v.c_str(), wv);
   if (overwrite || GetEnvironmentVariableW(wk.c_str(), NULL, 0) == 0)
   {
      SetEnvironmentVariableW(wk.c_str(), wv.c_str());
   }
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
   wchar_t *allenv = GetEnvironmentStringsW();
   wchar_t *curenv = allenv;
   String keyval, key, val;
   
   while (*curenv)
   {
      ToMultiByteString(curenv, UTF8Codepage, keyval);
      size_t pos = keyval.find('=');
      
      if (pos != std::string::npos)
      {
         if (pos == 0)
         { 
            if (mVerbose)
            {
               Log::PrintInfo("[gcore] Env::asDict: Ignore env string \"%s\"", keyval.c_str());
            }
         }
         else
         {
            key = keyval.substr(0, pos);
            val = keyval.substr(pos+1);
            
            if (key.length() > 0)
            {
               d[key] = val;
            }
         }
      }
      curenv += wcslen(curenv) + 1;
   }
   FreeEnvironmentStringsW(allenv);
#endif
   return d.size();
}

} // gcore
