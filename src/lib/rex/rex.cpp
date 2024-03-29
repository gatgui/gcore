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

#include <gcore/rex.h>
#include <gcore/log.h>
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

bool RexMatch::hasNamedGroup(const std::string &n) const
{
  std::map<std::string, size_t>::const_iterator it = mNamedGroups.find(n);
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

String RexMatch::group(const std::string &n) const
{
  std::map<std::string, size_t>::const_iterator it = mNamedGroups.find(n);
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
  return mGroups[i].first;
}

size_t RexMatch::length(size_t i) const
{
  if (!hasGroup(i))
  {
    return 0;
  }
  return (mGroups[i].second - mGroups[i].first);
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
  mValid = (mCode != 0);
  mNumGroups = (mValid ? info.numGroups : 0);
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
  
  //MatchInfo info(s.c_str(), s.c_str()+s.length(), flags, mNumGroups+1);
  
  //const char *beg = info.beg + offset;
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
  
  //while (cur < info.end)
  //while (cur < end)
  do
  {
#ifdef _DEBUG_REX
    Log::PrintDebug("[gcore] Rex::search: Try match with \"%s\"", cur);
#endif
    MatchInfo info(s.c_str(), s.c_str()+s.length(), flags, mNumGroups+1);
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
      Log::SetIndentLevel(Log::GetIndentLevel()+1);
      Log::PrintDebug("Matched string: \"%s\"", m.mStr.c_str());
      Log::PrintDebug("Matched range: [%d, %d]", m.mRange.first, m.mRange.second);
      for (size_t i=0; i<m.mGroups.size(); ++i)
      {
        Log::PrintDebug("Matched group %d: [%d, %d] \"%s\"", i, m.mGroups[i].first, m.mGroups[i]/second, m.group(i).c_str());
      }
      Log::PrintDebug("Pre: \"%s\"", m.pre().c_str());
      Log::PrintDebug("Post: \"%s\"", m.post().c_str());
      Log::SetIndentLevel(Log::GetIndentLevel()-1);
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

bool Rex::match(const String &s, unsigned short flags, size_t offset, size_t len) const
{
  RexMatch m;
  return match(s, m, flags, offset, len);
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
            std::string name;
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

}
