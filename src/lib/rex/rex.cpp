/*

Copyright (C) 2010  Gaetan Guidet

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
#include "parse.h"
#include "instruction.h"

namespace gcore {

_RawString::_RawString(const char *s)
  : e(s) {
  e.erase(0,1);
  e.erase(e.length()-1,1);
}

_RawString::operator String () const {
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
    mGroups = rhs.mGroups;
    mNamedGroups = rhs.mNamedGroups;
  }
  return *this;
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
  return mGroups[i].first - mRange.first;
}

size_t RexMatch::length(size_t i) const
{
  if (!hasGroup(i))
  {
    return 0;
  }
  return (mGroups[i].second - mGroups[i].first + 1);
}

size_t RexMatch::numGroups() const
{
  return mGroups.size();
}

// ---

Rex::Rex()
  : mValid(false), mCode(0), mNumGroups(0)
{
}

Rex::Rex(const String &exp)
  : mValid(false), mCode(0), mNumGroups(0)
{
  set(exp);
}

Rex::Rex(const Rex &rhs)
  : mValid(false), mCode(0), mNumGroups(0)
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

void Rex::set(const String &exp)
{
  ParseInfo info;
  info.numGroups = 0;
  
  mExp = exp;
  
  const char *pc = mExp.c_str();
  
  mCode = ParseExpression(&pc, info);
  
  mNumGroups = info.numGroups;
}

const String Rex::get() const
{
  return mExp;
}

bool Rex::search(const String &s, RexMatch &m, unsigned short flags, size_t offset, size_t len) const
{
  if (!mCode)
  {
    return false;
  }
  
  if (len == size_t(-1))
  {
    len = s.length();
  }
  
  if (offset >= s.length())
  {
    return false;
  }
  
  if (offset+len > s.length())
  {
    return false;
  }
  
  //MatchInfo info(s.c_str()+offset, s.c_str()+offset+len, flags, mNumGroups+1);
  //const char *cur = info.beg;
  
  MatchInfo info(s.c_str(), s.c_str()+s.length(), flags, mNumGroups+1);
  
  const char *beg = info.beg + offset;
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
  
  //while (cur < info.end)
  //while (cur < end)
  do
  {
#ifdef _DEBUG
    std::cout << "Try match with \"" << cur << "\"" << std::endl;
#endif
    const char *rv = code->match(cur, info);
    if (rv != 0)
    {
      std::swap(info.gmatch, m.mGroups);
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
#ifdef _DEBUG
      std::cout << "  Matched string: \"" << m.mStr << "\"" << std::endl;
      std::cout << "  Matched range: [" << m.mRange.first << ", " << m.mRange.second << "]" << std::endl;
      for (size_t i=0; i<m.mGroups.size(); ++i)
      {
        std::cout << "  Matched group " << i << ": [" << m.mGroups[i].first << ", " << m.mGroups[i].second << "]: \"" << m.group(i) << "\"" << std::endl;
      }
      std::cout << "  Pre: \"" << m.pre() << "\"" << std::endl;
      std::cout << "  Post: \"" << m.post() << "\"" << std::endl;
#endif
      return true;
    }
    //++cur;
    cur += step;
  } while (cur >= beg && cur < end);
  
  return false;
}

bool Rex::search(const String &s, unsigned short flags, size_t offset, size_t len) const
{
  RexMatch m;
  return search(s, m, flags, offset, len);
}

bool Rex::match(const String &s, RexMatch &m, unsigned short flags, size_t offset, size_t len) const
{
  if (!mCode)
  {
    return false;
  }
  
  if (len == size_t(-1))
  {
    len = s.length();
  }
  
  if (offset >= s.length())
  {
    return false;
  }
  
  if (offset+len > s.length())
  {
    return false;
  }
  
  //MatchInfo info(s.c_str()+offset, s.c_str()+offset+len, flags, mNumGroups+1);
  //const char *cur = info.beg;
  MatchInfo info(s.c_str(), s.c_str()+s.length(), flags, mNumGroups+1);
  
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
  
  const char *rv = mCode->match(cur, info);
  if (rv != 0)
  {
    std::swap(info.gmatch, m.mGroups);
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

bool Rex::match(const String &s, unsigned short flags, size_t offset, size_t len) const
{
  RexMatch m;
  return match(s, m, flags, offset, len);
}

String Rex::substitute(const RexMatch &m, const String &in, bool *failed) const
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
            if (failed)
            {
              *failed = true;
            }
            return "";
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
  
  if (failed)
  {
    *failed = true;
  }
  
  return "";
}

String Rex::substitute(const String &str, const String &in, bool *failed) const
{
  RexMatch m;
  if (search(str, m))
  {
    return substitute(m, in, failed);
  }
  if (failed)
  {
    *failed = true;
  }
  return "";
}

std::ostream& operator<<(std::ostream &os, const Rex &r)
{
  if (r.mCode != 0)
  {
    r.mCode->toStream(os);
  }
  return os;
}

}
