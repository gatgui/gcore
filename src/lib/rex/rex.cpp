/*

Copyright (C) 2010~  Gaetan Guidet

This file is part of gcore.

rex is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at
your option) any later version.

rex is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
USA.

*/

#include <gcore/rex.h>
#include <gcore/log.h>
#include "parse.h"
#include "instruction.h"

namespace gcore
{

_RawString::_RawString(const char *s)
   : e(s)
{
   e.erase(0,1);
   e.erase(e.length()-1,1);
}

_RawString::operator String () const
{
   return e;
}

RexMatch::RexMatch()
   : mStr("")
{
}

RexMatch::RexMatch(const RexMatch &rhs)
{
   operator=(rhs);
}

RexMatch::~RexMatch()
{
}

RexMatch& RexMatch::operator=(const RexMatch &rhs)
{
   if (this != &rhs)
   {
      mStr = rhs.mStr;
      mRange = rhs.mRange;
      mGroups = rhs.mGroups;
      mNamedGroups = rhs.mNamedGroups;
   }
   return *this;
}

void RexMatch::clear()
{
   mStr = "";
   mRange.first = -1;
   mRange.second = -1;
   mGroups.clear();
   mNamedGroups.clear();
}

bool RexMatch::hasGroup(size_t i) const
{
   return (i < mGroups.size() && mGroups[i].first>=0 && mGroups[i].second>=0);
}

bool RexMatch::hasNamedGroup(const String &n) const
{
   std::map<String, size_t>::const_iterator it = mNamedGroups.find(n);
   if (it == mNamedGroups.end())
   {
      return false;
   }
   size_t i = it->second;
   return hasGroup(i);
}

String RexMatch::pre() const
{
   //return mStr.substr(mRange.first, mGroups[0].first - mRange.first + 1);
   return mStr.substr(0, mRange.first);
}

String RexMatch::post() const
{
   //return mStr.substr(mGroups[0].second + 1, mRange.second - mGroups[0].second);
   return mStr.substr(mRange.second);
}

String RexMatch::group(size_t i) const
{
   if (!hasGroup(i))
   {
      return "";
   }
   return mStr.substr(mGroups[i].first, mGroups[i].second - mGroups[i].first);
}

String RexMatch::group(const String &n) const
{
   std::map<String, size_t>::const_iterator it = mNamedGroups.find(n);
   if (it == mNamedGroups.end())
   {
      return "";
   }
   size_t i = it->second;
   if (!hasGroup(i))
   {
      return "";
   }
   return mStr.substr(mGroups[i].first, mGroups[i].second - mGroups[i].first);
}

size_t RexMatch::offset(size_t i) const
{
   if (!hasGroup(i))
   {
      return 0;
   }
   return mGroups[i].first; // - mRange.first;
}

size_t RexMatch::length(size_t i) const
{
   if (!hasGroup(i))
   {
      return 0;
   }
   return (mGroups[i].second - mGroups[i].first);
}

size_t RexMatch::groupCount() const
{
   return mGroups.size();
}

// ---

Rex::Rex()
   : mValid(false)
   , mCode(0)
   , mNumGroups(0)
{
}

Rex::Rex(const String &s)
   : mValid(false)
   , mCode(0)
   , mNumGroups(0)
{
   set(s);
}

Rex::Rex(const Rex &rhs)
   : mValid(false)
   , mCode(0)
   , mNumGroups(0)
{
   operator=(rhs);
}

Rex::~Rex()
{
   if (mCode)
   {
      delete mCode;
   }
}

Rex& Rex::operator=(const Rex &rhs)
{
   if (this != &rhs)
   {
      mValid = rhs.mValid;
      mExp = rhs.mExp;
      mNumGroups = rhs.mNumGroups;
      if (mCode)
      {
         delete mCode;
      }
      //mCode = rhs.mCode->clone();
      mCode = Instruction::CloneList(rhs.mCode);
   }
   return *this;
}

bool Rex::valid() const
{
   return mValid;
}

void Rex::set(const String &s)
{
   ParseInfo info;
   
   mExp = s;
   
   info.numGroups = 0;
   info.beg = mExp.c_str();
   info.len = mExp.length();
   info.end = info.beg + info.len;
   
   const char *pc = info.beg;
   
   mCode = ParseExpression(&pc, info);
   
   mNumGroups = info.numGroups;
}

const String Rex::get() const
{
   return mExp;
}

bool Rex::search(const String &s, RexMatch &m, unsigned short flags) const
{
   return search(s, 0, s.length(), m, flags);
}

bool Rex::search(const String &s, size_t offset, RexMatch &m, unsigned short flags) const
{
   return search(s, offset, s.length(), m, flags);
}

bool Rex::search(const String &s, size_t offset, size_t len, RexMatch &m, unsigned short flags) const
{
   if (!mCode)
   {
      return false;
   }
   
   size_t slen = s.length();
   
   if (offset >= slen)
   {
      return false;
   }
   
   if (len > (slen - offset))
   {
      len = slen - offset;
   }
   
   const char *beg = s.c_str() + offset;
   const char *end = beg + len;
   const char *cur = beg;
   int step = 1;
   Instruction *code = mCode;
   
   if (flags & Rex::Reverse)
   {
      cur = end;
      step = -1;
      while (code->next())
      {
         code = code->next();
      }
   }
   
   m.clear();
   
   do
   {
#ifdef _DEBUG_REX
      Log::PrintDebug("[gcore] Rex::search: Try match with \"%s\"", cur);
#endif
      MatchInfo info(s.c_str(), s.c_str()+slen, flags, mNumGroups+1);
      const char *rv = code->match(cur, info);
      if (rv != 0)
      {
         std::swap(info.gmatch, m.mGroups);
         std::swap(info.gnames, m.mNamedGroups);
         if (flags & Rex::Reverse)
         {
            m.mRange.first = int(rv - info.beg);
            m.mRange.second = int(cur - info.beg);
         }
         else
         {
            m.mRange.first = int(cur - info.beg);
            m.mRange.second = int(rv - info.beg);
         }
         m.mGroups[0].first = m.mRange.first;
         m.mGroups[0].second = m.mRange.second;
         m.mStr = s;
#ifdef _DEBUG_REX
         Log::SetIndentLevel(Log::IndentLevel()+1);
         Log::PrintDebug("Matched string: \"%s\"", m.mStr.c_str());
         Log::PrintDebug("Matched range: [%d, %d]", m.mRange.first, m.mRange.second);
         for (size_t i=0; i<m.mGroups.size(); ++i)
         {
            Log::PrintDebug("Matched group %d: [%d, %d] \"%s\"", i, m.mGroups[i].first, m.mGroups[i].second, m.group(i).c_str());
         }
         Log::PrintDebug("Pre: \"%s\"", m.pre().c_str());
         Log::PrintDebug("Post: \"%s\"", m.post().c_str());
         Log::SetIndentLevel(Log::IndentLevel()-1);
#endif
         return true;
      }
      cur += step;
   } while (beg <= cur && cur < end);
   
   return false;
}

bool Rex::search(const String &s, unsigned short flags) const
{
   RexMatch m;
   return search(s, 0, s.length(), m, flags);
}

bool Rex::search(const String &s, size_t offset, unsigned short flags) const
{
   RexMatch m;
   return search(s, offset, s.length(), m, flags);
}

bool Rex::search(const String &s, size_t offset, size_t len, unsigned short flags) const
{
   RexMatch m;
   return search(s, offset, len, m, flags);
}

bool Rex::match(const String &s, RexMatch &m, unsigned short flags) const
{
   return match(s, 0, s.length(), m, flags);
}

bool Rex::match(const String &s, size_t offset, RexMatch &m, unsigned short flags) const
{
   return match(s, offset, s.length(), m, flags);
}

bool Rex::match(const String &s, size_t offset, size_t len, RexMatch &m, unsigned short flags) const
{
   if (!mCode)
   {
      return false;
   }
   
   size_t slen = s.length();
   
   if (offset >= slen)
   {
      return false;
   }
   
   if (len > (slen - offset))
   {
      len = slen - offset;
   }
   
   MatchInfo info(s.c_str(), s.c_str()+slen, flags, mNumGroups+1);
   
   const char *cur = info.beg + offset;
   Instruction *code = mCode;
   
   if (flags & Rex::Reverse)
   {
      cur += len;
      while (code->next())
      {
         code = code->next();
      }
   }
   
   m.clear();
   
   const char *rv = mCode->match(cur, info);
   if (rv != 0)
   {
      std::swap(info.gmatch, m.mGroups);
      std::swap(info.gnames, m.mNamedGroups);
      if (flags & Rex::Reverse)
      {
         m.mRange.first = int(rv - info.beg);
         m.mRange.second = int(cur - info.beg);
      }
      else
      {
         m.mRange.first = int(cur - info.beg);
         m.mRange.second = int(rv - info.beg);
      }
      m.mGroups[0].first = m.mRange.first;
      m.mGroups[0].second = m.mRange.second;
      m.mStr = s;
      return true;
   }
   else
   {
      return false;
   }
}

bool Rex::match(const String &s, unsigned short flags) const
{
   RexMatch m;
   return match(s, 0, s.length(), m, flags);
}

bool Rex::match(const String &s, size_t offset, unsigned short flags) const
{
   RexMatch m;
   return match(s, offset, s.length(), m, flags);
}

bool Rex::match(const String &s, size_t offset, size_t len, unsigned short flags) const
{
   RexMatch m;
   return match(s, offset, len, m, flags);
}

String Rex::substitute(const RexMatch &m, const String &in) const
{
   if (m.hasGroup(0))
   {
      String rv = "";
      
      const char *c = in.c_str();
      
      while (*c != '\0')
      {
         if (*c == '\\')
         {
            ++c;
            if (*c >= '0' && *c <= '9')
            {
               int grp = *c - '0';
               if (!m.hasGroup(grp))
               {
                  return in;
               }
               rv += m.group(grp);
            }
            else if (*c == '`')
            {
               rv += m.pre();
            }
            else if (*c == '&')
            {
               rv += m.group(0);
            }
            else if (*c == '\'')
            {
               rv += m.post();
            }
            else if (*c == 'g')
            {
               ++c;
               if (*c != '<')
               {
                  rv.append(1, *(c-2));
                  rv.append(1, *(c-1));
                  rv.append(1, *c);
               }
               else
               {
                  String name;
                  ++c;
                  while (*c != '>')
                  {
                     if (*c == '\0')
                     {
                        return in;
                     }
                     name.push_back(*c);
                     ++c;
                  }
                  if (!m.hasNamedGroup(name))
                  {
                     return in;
                  }
                  rv += m.group(name);
               }
            }
            else
            {
               --c;
               rv.append(1, *c);
            }
         }
         else
         {
            rv.append(1, *c);
         }
         ++c;
      }
      
      return rv;
   }
   
   return in;
}

String Rex::substitute(const String &in, const String &by, int maxCount) const
{
   if (maxCount == 0)
   {
      return in;
   }
   RexMatch m;
   int count = 0;
   String rv = in;
   while (search(rv, m))
   {
      rv = m.pre() + substitute(m, by) + m.post();
      if (++count == maxCount)
      {
         break;
      }
   }
   return rv;
}

std::ostream& operator<<(std::ostream &os, const Rex &r)
{
   if (r.mCode != 0)
   {
      r.mCode->toStream(os);
   }
   return os;
}

} // gcore
