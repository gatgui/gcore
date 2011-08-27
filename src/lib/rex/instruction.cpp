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

#include "instruction.h"
#include "charclass.h"
#include <gcore/rex.h>

namespace gcore {

#ifdef _WIN32
# define strncasecmp _strnicmp
# define strcasecmp _stricmp
#endif

MatchInfo::MatchInfo()
  : beg(0), end(0), flags(0), once(false)
{
}

MatchInfo::MatchInfo(const char *b, const char *e, unsigned short f, size_t ngroups)
  : beg(b), end(e), flags(f), once(false)
{
  gmatch.resize(ngroups, std::pair<int,int>(-1,-1));
}

MatchInfo::MatchInfo(const MatchInfo &rhs)
  : beg(rhs.beg)
  , end(rhs.end)
  , flags(rhs.flags)
  , gmatch(rhs.gmatch)
  , fstack(rhs.fstack)
  , cstack(rhs.cstack)
  , once(rhs.once)
  , gclosed(rhs.gclosed)
  , gnames(rhs.gnames)
{
  
}

MatchInfo& MatchInfo::operator=(const MatchInfo &rhs)
{
  if (this != &rhs)
  {
    beg = rhs.beg;
    end = rhs.end;
    flags = rhs.flags;
    gmatch = rhs.gmatch;
    fstack = rhs.fstack;
    cstack = rhs.cstack;
    once = rhs.once;
    gclosed = rhs.gclosed;
    gnames = rhs.gnames;
  }
  return *this;
}

// ---

Instruction::Instruction()
  : mNext(0), mPrev(0), mGroup(0)
{
}

Instruction::~Instruction()
{
  if (mNext)
  {
    delete mNext;
  }
}

void Instruction::toStream(std::ostream &os, const std::string &indent) const
{
  if (mNext)
  {
    mNext->toStream(os, indent);
  }
}

void Instruction::setGroup(Group *g)
{
  mGroup = g;
  if (mNext)
  {
    mNext->setGroup(g);
  }
}

void Instruction::setNext(Instruction *inst)
{
  if (mNext)
  {
    delete mNext;
  }
  mNext = inst;
  if (mNext)
  {
    mNext->setPrev(this);
  }
}

void Instruction::setPrev(Instruction *inst)
{
  mPrev = inst;
}

Group* Instruction::group() const
{
  return mGroup;
}

Instruction* Instruction::next() const
{
  return mNext;
}

Instruction* Instruction::prev() const
{
  return mPrev;
}

bool Instruction::preStep(const char *&cur, MatchInfo &info) const
{
  if (info.flags & Rex::Reverse)
  {
    if (--cur < info.beg)
    {
      return false;
    }
  }
  else
  {
    if (cur >= info.end)
    {
      return false;
    }
  }
  return true;
}

const char* Instruction::postStep(const char *cur, MatchInfo &info) const
{
  // if (consume ^ reverse) ++input;
  //Instruction *remain = 0;
  
  if (!(info.flags & Rex::Reverse))
  {
    ++cur;
    //remain = mPrev;
  }
  //else
  //{
  //  remain = mNext;
  //}
  //return (remain ? remain->match(cur, info) : cur);
  
  return (info.once ? cur : matchRemain(cur, info));
  //return matchRemain(cur, info);
}

const char* Instruction::matchRemain(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match remaining (" << typeid(*this).name() << "): \"" << cur << "\"" << std::endl;
#endif
  
  register bool failed = false;
  const char *rv = cur;
  
  if (info.flags & Rex::Reverse)
  {
    if (mPrev)
    {
      rv = mPrev->match(cur, info);
      failed = (rv == 0);
    }
  }
  else
  {
    if (mNext)
    {
      rv = mNext->match(cur, info);
      failed = (rv == 0);
    }
  }

#ifdef _DEBUG
  if (mGroup)
  {
    std::cout << "Is in group " << (int)mGroup->index() << std::endl;
  }
#endif
  
  if (mGroup && info.gclosed[mGroup] == false)
  {
#ifdef _DEBUG
    std::cout << "Group " << (int)mGroup->index() << " not closed yet" << std::endl;
#endif
    // is it?
    // note on groups:
    //   inverted groups are necessarily zero width groups, meaning
    //   the rv pointer will be always restored to the position at the begining of the group
    
    unsigned short gflags = info.fstack.back();
    const char *gcur = 0;
    if (mGroup->zeroWidth())
    {
      gcur = info.cstack.back();
    }
    
    rv = (rv == 0 ? cur : rv);
    if (mGroup->end(failed, rv, info))
    {
      rv = mGroup->matchRemain(rv, info);
      if (!rv)
      {
#ifdef _DEBUG
        std::cout << "Re-open matched group " << mGroup->index() << std::endl;
#endif
        info.flags = gflags;
        mGroup->open(gcur, info);
      }
      return rv;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    return rv;
  }
}

Instruction* Instruction::CloneList(Instruction *i)
{
  if (!i)
  {
    return 0;
  }
  Instruction *ci = i->clone();
  Instruction *cni = ci;
  Instruction *ni = i->next();
  while (ni)
  {
    cni->setNext(ni->clone());
    cni = cni->next();
    ni = ni->next();
  }
  return ci;
}

// ---

Single::Single(char c)
  : Instruction(), mChar(c), mUpperChar(c), mLowerChar(c)
{
  int diff = 'A' - 'a';
  if (CHAR_IS(c, LOWER_CHAR))
  {
    mUpperChar = (char)(c + diff);
  }
  else if (CHAR_IS(c, UPPER_CHAR))
  {
    mLowerChar = (char)(c - diff);
  }
}

Single::~Single()
{
}

Instruction* Single::clone() const
{
  return new Single(mChar);
}

const char* Single::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match single character \'" << mChar << "\' with \"" << cur << "\"... ";
#endif
  
  register bool matched;
  
  if (preStep(cur, info))
  {
    matched = (info.flags & Rex::NoCase) ? (*cur == mLowerChar || *cur == mUpperChar) : (*cur == mChar);
    
    if (matched)
    {
#ifdef _DEBUG
      std::cout << "OK (Single)" << std::endl;
#endif
      return postStep(cur, info);
    }
  }
  
#ifdef _DEBUG
  std::cout << "Failed" << std::endl;
#endif
  return 0;
}

void Single::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << "Single \'" << mChar << "\'" << std::endl;
  Instruction::toStream(os, indent);
}

// ---

Any::Any()
  :Instruction()
{
}

Any::~Any()
{
}

Instruction* Any::clone() const
{
  return new Any();
}

const char* Any::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match any character... ";
#endif
  
  if (preStep(cur, info))
  {
    if ((info.flags & Rex::DotMatchNewline) || (*cur != '\r' && *cur != '\n'))
    {
#ifdef _DEBUG
      std::cout << "OK (Any)" << std::endl;
#endif
      return postStep(cur, info);
    }
  }

#ifdef _DEBUG
  std::cout << "Failed" << std::endl;
#endif
  
  return 0;
}

void Any::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << "Any" << std::endl;
  Instruction::toStream(os, indent);
}

// ---

Word::Word(bool invert)
  : Instruction(), mInvert(invert)
{
}

Word::~Word()
{
}

Instruction* Word::clone() const
{
  return new Word(mInvert);
}

const char* Word::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match " << (mInvert ? "non " : "") << "word character... ";
#endif
  if (preStep(cur, info))
  {
    if (mInvert ^ CHAR_IS(*cur, WORD_CHAR))
    {
#ifdef _DEBUG
      std::cout << "OK (Word)" << std::endl;
#endif
      return postStep(cur, info);
    }
  }
#ifdef _DEBUG
  std::cout << "Failed" << std::endl;
#endif
  return 0;
}

void Word::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << (mInvert ? "!Word" : "Word") << std::endl;
  Instruction::toStream(os, indent);
}

// ---


Digit::Digit(bool invert)
  : Instruction(), mInvert(invert)
{
}

Digit::~Digit()
{
}

Instruction* Digit::clone() const
{
  return new Digit(mInvert);
}

const char* Digit::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match " << (mInvert ? "non " : "") << "digit character... ";
#endif
  if (preStep(cur, info))
  {
    if (mInvert ^ CHAR_IS(*cur, DIGIT_CHAR))
    {
#ifdef _DEBUG
      std::cout << "OK (Digit)" << std::endl;
#endif
      return postStep(cur, info);
    }
  }
#ifdef _DEBUG
  std::cout << "Failed" << std::endl;
#endif
  return 0;
}

void Digit::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << (mInvert ? "!Digit" : "Digit") << std::endl;
  Instruction::toStream(os, indent);
}

// ---

LowerLetter::LowerLetter()
  : Instruction()
{
}

LowerLetter::~LowerLetter()
{
}

Instruction* LowerLetter::clone() const
{
  return new LowerLetter();
}

const char* LowerLetter::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match lower case letter... ";
#endif
  if (preStep(cur, info))
  {
    if (CHAR_IS(*cur, LOWER_CHAR) || ((info.flags & Rex::NoCase) && CHAR_IS(*cur, UPPER_CHAR)))
    {
#ifdef _DEBUG
      std::cout << "OK (LowerLetter)" << std::endl;
#endif
      return postStep(cur, info);
    }
  }
#ifdef _DEBUG
  std::cout << "Failed" << std::endl;
#endif
  return 0;
}

void LowerLetter::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << "LowerLetter" << std::endl;
  Instruction::toStream(os, indent);
}

// ---

UpperLetter::UpperLetter()
  : Instruction()
{
}

UpperLetter::~UpperLetter()
{
}

Instruction* UpperLetter::clone() const
{
  return new UpperLetter();
}

const char* UpperLetter::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match upper case letter... ";
#endif
  if (preStep(cur, info))
  {
    if (CHAR_IS(*cur, UPPER_CHAR) || ((info.flags & Rex::NoCase) && CHAR_IS(*cur, LOWER_CHAR)))
    {
#ifdef _DEBUG
      std::cout << "OK (UpperLetter)" << std::endl;
#endif
      return postStep(cur, info);
    }
  }
#ifdef _DEBUG
  std::cout << "Failed" << std::endl;
#endif
  return 0;
}

void UpperLetter::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << "UpperLetter" << std::endl;
  Instruction::toStream(os, indent);
}

// ---

Letter::Letter(bool invert)
  : Instruction(), mInvert(invert)
{
}

Letter::~Letter()
{
}

Instruction* Letter::clone() const
{
  return new Letter(mInvert);
}

const char* Letter::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match " << (mInvert ? "non " : "") << "letter character... ";
#endif
  if (preStep(cur, info))
  {
    if (mInvert ^ CHAR_IS(*cur, LETTER_CHAR))
    {
#ifdef _DEBUG
      std::cout << "OK (Letter)" << std::endl;
#endif
      return postStep(cur, info);
    }
  }
#ifdef _DEBUG
  std::cout << "Failed" << std::endl;
#endif
  return 0;
}

void Letter::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << (mInvert ? "!Letter" : "Letter") << std::endl;
  Instruction::toStream(os, indent);
}

// ---

Hexa::Hexa(bool invert)
  : Instruction(), mInvert(invert)
{
}

Hexa::~Hexa()
{
}

Instruction* Hexa::clone() const
{
  return new Hexa(mInvert);
}

const char* Hexa::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match " << (mInvert ? "non " : "") << "hexa character... ";
#endif
  if (preStep(cur, info))
  {
    if (mInvert ^ CHAR_IS(*cur, HEXA_CHAR))
    {
#ifdef _DEBUG
      std::cout << "OK (Hexa)" << std::endl;
#endif
      return postStep(cur, info);
    }
  }
#ifdef _DEBUG
  std::cout << "Failed" << std::endl;
#endif
  return 0;
}

void Hexa::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << (mInvert ? "!Hexa" : "Hexa") << std::endl;
  Instruction::toStream(os, indent);
}

// ---

Space::Space(bool invert)
  : Instruction(), mInvert(invert)
{
}

Space::~Space()
{
}

Instruction* Space::clone() const
{
  return new Space(mInvert);
}

const char* Space::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match " << (mInvert ? "non " : "") << "space character... ";
#endif
  if (preStep(cur, info))
  {
    if (mInvert ^ CHAR_IS(*cur, SPACE_CHAR))
    {
#ifdef _DEBUG
      std::cout << "OK (Space)" << std::endl;
#endif
      return postStep(cur, info);
    }
  }
#ifdef _DEBUG
  std::cout << "Failed" << std::endl;
#endif
  return 0;
}

void Space::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << (mInvert ? "!Space" : "Space") << std::endl;
  Instruction::toStream(os, indent);
}

// ---

CharRange::CharRange(char from, char to)
  : Instruction(), mFrom(from), mTo(to)
{
}

CharRange::~CharRange()
{
}

Instruction* CharRange::clone() const
{
  return new CharRange(mFrom, mTo);
}

void CharRange::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << "CharRange " << mFrom << "-" << mTo << std::endl;
  Instruction::toStream(os, indent);
}

const char* CharRange::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match character in range " << mFrom << "-" << mTo << "... ";
#endif
  register char cc;
  register int casediff = 0;
  register bool matched = false;
  
  if (preStep(cur, info))
  {
    if (info.flags & Rex::NoCase)
    {
      casediff = 'A' - 'a';
      cc = mFrom;
      while (cc <= mTo)
      {
        if ((*cur == cc) ||
            (CHAR_IS(cc, LOWER_CHAR) && (*cur == cc+casediff)) ||
            (CHAR_IS(cc, UPPER_CHAR) && (*cur == cc-casediff)))
        {
          matched = true;
          break;
        }
        ++cc;
      }
    }
    else
    {
      matched = (*cur >= mFrom && *cur <= mTo);
    }
    
    if (matched)
    {
#ifdef _DEBUG
      std::cout << "OK (CharRange)" << std::endl;
#endif
      return postStep(cur, info);
    }
  }
#ifdef _DEBUG
  std::cout << "Failed" << std::endl;
#endif
  return 0;
}

// ---


CharClass::CharClass(Instruction *klass, bool invert)
  : Instruction(), mInvert(invert), mFirst(klass)
{
}

CharClass::~CharClass()
{
  if (mFirst)
  {
    delete mFirst;
  }
}

Instruction* CharClass::clone() const
{
  return new CharClass(CloneList(mFirst), mInvert);
}

void CharClass::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << "CharClass[" << std::endl;
  if (mFirst)
  {
    mFirst->toStream(os, indent+"  ");
  }
  os << indent << "]" << std::endl;
  Instruction::toStream(os, indent);
}

const char* CharClass::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match character " << (mInvert ? "not " : "") << "in class... " << std::endl;
#endif
  const char *rv = 0;
  Instruction *inst = mFirst;
  
  info.once = true;
  while (inst)
  {
    rv = inst->match(cur, info);
    if (rv != 0)
    {
      break;
    }
    inst = inst->next();
  }
  info.once = false;
  
  if (mInvert)
  {
    if (rv == 0)
    {
      if (preStep(cur, info))
      {
#ifdef _DEBUG
        std::cout << "OK (character class)" << std::endl;
#endif
        return postStep(cur, info);
      }
    }
#ifdef _DEBUG
    std::cout << "Failed (character class)" << std::endl;
#endif
    return 0;
  }
  else
  {
    //return rv; // only?
    if (rv != 0)
    {
#ifdef _DEBUG
      std::cout << (rv ? "OK" : "Failed") << " (character class)" << std::endl;
#endif
      return matchRemain(rv, info);
    }
    return 0;
  }
}

// ---

Repeat::Repeat(Instruction *s, int min, int max, bool lazy)
  : Instruction(), mMin(min), mMax(max), mLazy(lazy), mInst(s)
{
}

Repeat::~Repeat()
{
  if (mInst)
  {
    delete mInst;
  }
}

void Repeat::setInstruction(Instruction *i)
{
  if (mInst)
  {
    delete mInst;
  }
  mInst = i;
}

void Repeat::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << "Repeat [" << mMin << ", ";
  if (mMax <= 0)
  {
    os << "inf";
  }
  else
  {
    os << mMax;
  }
  os << "] " << (mLazy ? "lazy" : "greedy") << " {" << std::endl;
  mInst->toStream(os, indent+"  ");
  os << indent << "}" << std::endl;
  Instruction::toStream(os, indent);
}

Instruction* Repeat::clone() const
{
  return new Repeat(mInst ? mInst->clone() : 0, mMin, mMax, mLazy);
}

const char* Repeat::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match repeat... ";
#endif
  register int count = 0;
  const char *rv = 0;
  
  if (!mInst)
  {
#ifdef _DEBUG
    std::cout << "Failed" << std::endl;
#endif
    return 0;
  }
  
#ifdef _DEBUG
  std::cout << std::endl;
#endif
  // match at leat mMin times (same for greedy and lazy)
  
  while (count < mMin)
  {
    rv = mInst->match(cur, info);
    if (!rv)
    {
#ifdef _DEBUG
      std::cout << "Failed" << std::endl;
#endif
      return 0;
    }
    cur = rv;
    ++count;
  }
  
  // now if lazy try match remaining... remaining is mNext (of mPrev)
  // repeat if no match
  
  unsigned short gflags = 0;
  const char *gcur = 0;
  
  if (mGroup)
  {
    gflags = info.fstack.back();
    if (mGroup->zeroWidth())
    {
      gcur = info.cstack.back();
    }
  }
  
  if (mLazy)
  {
    rv = matchRemain(cur, info);
    
    while (!rv && (mMax <= 0 || count < mMax))
    {
      // repeated instruction might be in a group
      // calling matchRemain may have closed the group
      if (mGroup && info.gclosed[mGroup] == true)
      {
        info.flags = gflags;
        mGroup->open(gcur, info);
      }
      
      rv = mInst->match(cur, info);
      if (!rv)
      {
#ifdef _DEBUG
        std::cout << "Failed" << std::endl;
#endif
        return 0;
      }
      else
      {
        cur = rv;
        ++count;
        rv = matchRemain(cur, info);
      }
    }
  }
  else
  {
    info.cstack.push_back(cur);
    
    // match repeated insttruction as most as possible
    while (cur && (mMax <= 0 || count < mMax))
    {
      cur = mInst->match(cur, info);
      if (cur)
      {
        info.cstack.push_back(cur);
        ++count;
      }
    }
    
    // backtrack
    while (count >= mMin)
    {
      cur = info.cstack.back();
      info.cstack.pop_back();
      --count;
      rv = matchRemain(cur, info);
      if (rv)
      {
        break;
      }
      else
      {
        // repeated instruction might be in a group
        // calling matchRemain may have closed the group
        
        if (mGroup && info.gclosed[mGroup] == true)
        {
#ifdef _DEBUG
          std::cout << "Repeat in group match remaining failed, re-open group " << mGroup->index() << std::endl;
#endif
          info.flags = gflags;
          mGroup->open(gcur, info);
        }
      }
    }
    
    // reset cur stack
    while (count >= mMin)
    {
      info.cstack.pop_back();
      --count;
    }
  }
#ifdef _DEBUG
  std::cout << (rv ? "OK (Repeat)" : "Failed") << std::endl;
#endif
  return rv;
}

// ---


Alternative::Alternative(Instruction *i0, Instruction *i1)
  : Instruction(), mFirst(i0), mSecond(i1)
{      
}

void Alternative::setGroup(class Group *grp)
{
  Instruction::setGroup(grp);
  if (mFirst)
  {
    mFirst->setGroup(grp);
  }
  if (mSecond)
  {
    mSecond->setGroup(grp);
  }
}

Alternative::~Alternative()
{
  if (mFirst)
  {
    delete mFirst;
  }
  if (mSecond)
  {
    delete mSecond;
  }
}

Instruction* Alternative::clone() const
{
  return new Alternative(CloneList(mFirst), CloneList(mSecond));
}

void Alternative::toStream(std::ostream &os, const std::string &indent) const
{
  if (mFirst)
  {
    mFirst->toStream(os, indent);
  }
  os << indent << "|" << std::endl;
  if (mSecond)
  {
    mSecond->toStream(os, indent);
  }
  Instruction::toStream(os, indent);
}

const char* Alternative::match(const char *cur, MatchInfo &info) const
{
  const char *rv = 0;
  //MatchInfo mi = info;
  if (mFirst)
  {
    rv = mFirst->match(cur, info);
  }
  if (rv)
  {
    return matchRemain(rv, info);
  }
  else if (mSecond)
  {
    //info = mi;
    rv = mSecond->match(cur, info);
  }
  return (rv ? matchRemain(rv, info) : 0);
}

// ---

Group::Group(int index, Instruction *first, bool zerowidth, bool invert,
             unsigned short flags, TriState nc, TriState ml, TriState dnl,
             const std::string &name)
  : Instruction()
  , mIndex(index)
  , mFirst(first)
  , mLast(0)
  , mZeroWidth(zerowidth)
  , mInvert(invert)
  , mDotNewline(dnl)
  , mNoCase(nc)
  , mMultiline(ml)
  , mFlags(flags)
  , mName(name)
{
  if (mFirst)
  {
    mFirst->setGroup(this);
    mLast = mFirst;
    while (mLast->next())
    {
      mLast = mLast->next();
    }
  }
}

Group::~Group()
{
  if (mFirst)
  {
    delete mFirst;
  }
}

bool Group::end(bool failure, const char *&cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Restore flags: " << info.flags << " -> " << info.fstack.back() << " (flags stack size = " << info.fstack.size() << ")" << std::endl;
#endif
  
  info.gclosed[this] = true;
  
  info.flags = info.fstack.back();
  info.fstack.pop_back();
  
  const char *c = 0;
  if (mZeroWidth)
  {
    c = info.cstack.back();
    info.cstack.pop_back();
  }
  
  if (failure ^ mInvert) // invert=0, failure=1 || invert=1, failure=0
  {
#ifdef _DEBUG
    std::cout << "Group end failed" << std::endl;
#endif
    return false;
  }
  
  if (mIndex > 0)
  {
    info.gnames[mName] = mIndex;
    if (info.flags & Rex::Reverse)
    {
      info.gmatch[mIndex].first = int(cur - info.beg);
    }
    else
    {
      info.gmatch[mIndex].second = int(cur - info.beg);
    }
#ifdef _DEBUG
    std::cout << "End capture " << mIndex+1 << " of " << info.gmatch.size() << ": [" << info.gmatch[mIndex].first << ", " << info.gmatch[mIndex].second << "]" << std::endl;
#endif
  }
  
  if (mZeroWidth)
  {
    cur = c;
  }
  /*
  if (mZeroWidth)
  {
#ifdef _DEBUG
    std::cout << "Zerowidth group, restore character pointer" << std::endl;
#endif
    cur = info.cstack.back();
    info.cstack.pop_back();
  }
  */
  
  return true;
}

Instruction* Group::clone() const
{
  return new Group(mIndex, CloneList(mFirst), mZeroWidth, mInvert,
                   mFlags, mNoCase, mMultiline, mDotNewline, mName);
}

void Group::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << "(";
  os << "group:" << mIndex << "[\"" << mName << "\"] ";
  
  if (mInvert)
  {
    os << "invert ";
  }
  
  if (mZeroWidth)
  {
    os << "zerowidth ";
  }
  
  switch (mNoCase)
  {
  case On:  os << "nocase[on] "; break;
  case Off: os << "nocase[off] "; break;
  case Inherit: os << "nocase[inherit] "; break;
  default: break;
  }
  
  switch (mMultiline)
  {
  case On:  os << "multiline[on] "; break;
  case Off: os << "multiline[off] "; break;
  case Inherit: os << "multiline[inherit] "; break;
  default: break;
  }
  
  switch (mDotNewline)
  {
  case On:  os << "dotmatchnewline[on] "; break;
  case Off: os << "dotmatchnewline[off] "; break;
  case Inherit: os << "dotmatchnewline[inherit] "; break;
  default: break;
  }
  
  if (mFlags & Rex::Reverse)
  {
    os << "reverse";
  }
  
  os << std::endl;
  
  if (mFirst)
  {
    mFirst->toStream(os, indent+"  ");
  }
  
  os << indent << ")" << std::endl;
  
  Instruction::toStream(os, indent);
}

void Group::open(const char *cur, MatchInfo &info) const
{
  
  //register unsigned short flags = mFlags;
  bool originallyReversed = ((info.flags & Rex::Reverse) != 0);
  
  register unsigned short flags = mFlags;
  if (originallyReversed)
  {
    // for lookbehind Rex::Reverse flag is alreay set in mFlags
    // should we invert it? what is a lookbehind in reverse mode?
    // -> a lookahead !
    // -> conclusion: invert Rex::Reverse flag
    if (flags & Rex::Reverse)
    {
      flags = (unsigned short)(flags & ~Rex::Reverse);
    }
    else
    {
      flags = (unsigned short)(flags | Rex::Reverse);
    }
  }
  
  if (mNoCase == On)
  {
    flags = (unsigned short)(flags | Rex::NoCase);
  }
  else if (mNoCase == Inherit)
  {
    flags = (unsigned short)(flags | (info.flags & Rex::NoCase));
  }
  
  if (mMultiline == On)
  {
    flags = (unsigned short)(flags | Rex::Multiline);
  }
  else if (mMultiline == Inherit)
  {
    flags = (unsigned short)(flags | (info.flags & Rex::Multiline));
  }
  
  if (mDotNewline == On)
  {
    flags = (unsigned short)(flags | Rex::DotMatchNewline);
  }
  else if (mDotNewline == Inherit)
  {
    flags = (unsigned short)(flags | (info.flags & Rex::DotMatchNewline));
  }
  
  if (mFirst)
  {
    info.gclosed[this] = false;
    
    info.fstack.push_back(info.flags);
    info.flags = flags;
    
    if (mZeroWidth)
    {
      info.cstack.push_back(cur);
    }
  }
  else
  {
    info.flags = flags;
  }
}

const char* Group::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match group (";
#endif
  
  //register unsigned short flags = mFlags;
  bool originallyReversed = ((info.flags & Rex::Reverse) != 0);
  
  register unsigned short flags = mFlags;
  if (originallyReversed)
  {
    // for lookbehind Rex::Reverse flag is alreay set in mFlags
    // should we invert it? what is a lookbehind in reverse mode?
    // -> a lookahead !
    // -> conclusion: invert Rex::Reverse flag
    if (flags & Rex::Reverse)
    {
      flags = (unsigned short)(flags & ~Rex::Reverse);
    }
    else
    {
      flags = (unsigned short)(flags | Rex::Reverse);
    }
  }
  
  if (mNoCase == On)
  {
    flags = (unsigned short)(flags | Rex::NoCase);
  }
  else if (mNoCase == Inherit)
  {
    flags = (unsigned short)(flags | (info.flags & Rex::NoCase));
  }
  
  if (mMultiline == On)
  {
    flags = (unsigned short)(flags | Rex::Multiline);
  }
  else if (mMultiline == Inherit)
  {
    flags = (unsigned short)(flags | (info.flags & Rex::Multiline));
  }
  
  if (mDotNewline == On)
  {
    flags = (unsigned short)(flags | Rex::DotMatchNewline);
  }
  else if (mDotNewline == Inherit)
  {
    flags = (unsigned short)(flags | (info.flags & Rex::DotMatchNewline));
  }
  
#ifdef _DEBUG
  std::cout << "group: " << mIndex << ", ";
  std::cout << "invert: " << mInvert << ", ";
  std::cout << "zerowidth: " << mZeroWidth << ", ";
  std::cout << "reverse: " << ((mFlags & Rex::Reverse) == 1) << ", ";
  std::cout << "nocase: " << mNoCase << ", ";
  std::cout << "multiline: " << mMultiline << ", ";
  std::cout << "dotmatchnewline: " << mDotNewline;
  std::cout << ")... ";
#endif
  
  if (mFirst)
  {
#ifdef _DEBUG
    std::cout << std::endl;
#endif
    
    // not enough, if group in a repeat need a counter?
    info.gclosed[this] = false;
    
    if (mIndex > 0)
    {
      if (originallyReversed)
      {
        info.gmatch[mIndex].second = int(cur - info.beg);
      }
      else
      {
        info.gmatch[mIndex].first = int(cur - info.beg);
      }
    }
    
    info.fstack.push_back(info.flags);
    
    if (mZeroWidth)
    {
      info.cstack.push_back(cur);
    }
    
    info.flags = flags;
    
    const char *rv = 0;
    
    //if (originallyReversed)
    if (info.flags & Rex::Reverse)
    {
      rv = mLast->match(cur, info);
    }
    else
    {
      rv = mFirst->match(cur, info); // will call end() and matchRemain()
    }
    
    if (info.gclosed[this] == false)
    {
#ifdef _DEBUG
      std::cout << "... group not yet closed: rv = 0x" << std::hex << (void*)rv << std::dec << std::endl;
#endif
      bool failed = (rv == 0);
      rv = failed ? cur : rv;
      if (end(failed, rv, info))
      {
#ifdef _DEBUG
        std::cout << "OK (Group)" << std::endl;
#endif
        return matchRemain(rv, info);
      }
      else
      {
        return 0;
      }
    }
    else
    {
#ifdef _DEBUG
      std::cout << (rv == 0 ? "Failed" : "OK (Group)") << std::endl;
#endif
      return rv;
    }
  }
  else
  {
    // a group without instruction is used to modify flags for
    // the remaining of the expression
    info.flags = flags;
#ifdef _DEBUG
    std::cout << "OK (Group, flag modify only)" << std::endl;
#endif
    return matchRemain(cur, info);
  }
}

// ---

Backsubst::Backsubst(int index)
  : Instruction(), mIndex(index)
{      
}

Backsubst::Backsubst(const std::string &n)
  : Instruction(), mIndex(-1), mName(n)
{
}

Backsubst::~Backsubst()
{
}

Instruction* Backsubst::clone() const
{
  return (mIndex > 0 ? new Backsubst(mIndex) : new Backsubst(mName));
}

void Backsubst::toStream(std::ostream &os, const std::string &indent) const
{
  //os << indent << "\\" << mIndex << std::endl;
  if (mName.length() > 0)
  {
    os << indent << "\\g<" << mName << ">" << std::endl;
  }
  else
  {
    os << indent << "\\" << mIndex << std::endl;
  }
  Instruction::toStream(os, indent);
}

const char* Backsubst::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match backsubstitution " << mIndex << "... ";
#endif
  
  size_t index = 0;
  
  if (mIndex > 0)
  {
    index = mIndex;
  }
  else
  {
    std::map<std::string, size_t>::iterator it = info.gnames.find(mName);
    if (it == info.gnames.end() || it->second <= 0)
    {
#ifdef _DEBUG
      std::cout << "Failed" << std::endl;
#endif
      return 0;
    }
    index = it->second;
  }
  
  if (index >= info.gmatch.size())
  {
#ifdef _DEBUG
    std::cout << "Failed (invalid group index)" << std::endl;
#endif
    return 0;
  }
  
  if (info.gmatch[index].first < 0 || info.gmatch[index].second < 0)
  {
#ifdef _DEBUG
    std::cout << "Failed (group not matched)" << std::endl;
#endif
    return 0;
  }
  
  size_t bslen = info.gmatch[index].second - info.gmatch[index].first;
  
  if (info.flags & Rex::Reverse)
  {
    size_t rlen = cur - info.beg;
    
    if (rlen < bslen)
    {
#ifdef _DEBUG
      std::cout << "Failed" << std::endl;
#endif
      return 0;
    }
    
    const char *to = cur - bslen;
    const char *bs = info.beg + info.gmatch[index].second;
    
    --bs;
    --cur;
    
    if (info.flags & Rex::NoCase)
    {
      while (cur >= to)
      {
        if (CHAR_IS(*bs, LETTER_CHAR))
        {
          if (!CHAR_IS(*cur, LETTER_CHAR))
          {
#ifdef _DEBUG
            std::cout << "Failed" << std::endl;
#endif
            return 0;
          }
          
          size_t i0 = CHAR_IS(*cur, LOWER_CHAR) ? *cur - 'a' : *cur - 'A';
          size_t i1 = CHAR_IS(*bs, LOWER_CHAR) ? *bs - 'a' : *bs - 'A';
          
          if (i0 != i1)
          {
#ifdef _DEBUG
            std::cout << "Failed" << std::endl;
#endif
            return 0;
          }
        }
        else if (*cur != *bs)
        {
#ifdef _DEBUG
          std::cout << "Failed" << std::endl;
#endif
          return 0;
        }
        --bs;
        --cur;
      }
#ifdef _DEBUG
      std::cout << "OK (Backsubst)" << std::endl;
#endif
      return matchRemain(cur+1, info);
    }
    else
    {
      while (cur >= to)
      {
        if (*cur != *bs)
        {
#ifdef _DEBUG
          std::cout << "Failed" << std::endl;
#endif
          return 0;
        }
        --bs;
        --cur;
      }
#ifdef _DEBUG
      std::cout << "OK (Backsubst)" << std::endl;
#endif
      return matchRemain(cur+1, info);
    }
  }
  else
  {
    size_t rlen = info.end - cur;
    
    if (rlen < bslen)
    {
#ifdef _DEBUG
      std::cout << "Failed" << std::endl;
#endif
      return 0;
    }
    
    if (info.flags & Rex::NoCase)
    {
      if (strncasecmp(cur, info.beg + info.gmatch[index].first, bslen) == 0)
      {
#ifdef _DEBUG
        std::cout << "OK (Backsubst)" << std::endl;
#endif
        return matchRemain(cur + bslen, info);
      }
#ifdef _DEBUG
      std::cout << "Failed" << std::endl;
#endif
      return 0;
    }
    else
    {
      if (strncmp(cur, info.beg + info.gmatch[index].first, bslen) == 0)
      {
#ifdef _DEBUG
        std::cout << "OK (Backsubst)" << std::endl;
#endif
        return matchRemain(cur + bslen, info);
      }
#ifdef _DEBUG
      std::cout << "Failed" << std::endl;
#endif
      return 0;
    }
  }

#ifdef _DEBUG
  std::cout << "Failed" << std::endl;
#endif
  return 0;
}

// ---

WordStart::WordStart()
  : Instruction()
{
}

WordStart::~WordStart()
{
}

Instruction* WordStart::clone() const
{
  return new WordStart();
}

void WordStart::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << "WordStart" << std::endl;
  Instruction::toStream(os, indent);
}

const char* WordStart::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match word start... ";
#endif
  // same in reverse mode?
  if (CHAR_IS(*cur, WORD_CHAR) && ((cur <= info.beg) || !CHAR_IS(*(cur-1), WORD_CHAR)))
  {
#ifdef _DEBUG
    std::cout << "OK (WordStart)" << std::endl;
#endif
    return matchRemain(cur, info);
  }
#ifdef _DEBUG
  std::cout << "Failed" << std::endl;
#endif
  return 0;
}

// ---

WordEnd::WordEnd()
  : Instruction()
{
}

WordEnd::~WordEnd()
{
}

Instruction* WordEnd::clone() const
{
  return new WordEnd();
}

void WordEnd::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << "WordEnd" << std::endl;
  Instruction::toStream(os, indent);
}

const char* WordEnd::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match word end... ";
#endif
  // same in reverse mode?
  if (!CHAR_IS(*cur, WORD_CHAR) && ((cur <= info.beg) || CHAR_IS(*(cur-1), WORD_CHAR)))
  {
#ifdef _DEBUG
    std::cout << "OK (WordEnd)" << std::endl;
#endif
    return matchRemain(cur, info);
  }
#ifdef _DEBUG
  std::cout << "Failed" << std::endl;
#endif
  return 0;
}

// ---

WordBound::WordBound(bool invert)
  : Instruction(), mInvert(invert)
{
}

WordBound::~WordBound()
{
}

Instruction* WordBound::clone() const
{
  return new WordBound(mInvert);
}

void WordBound::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << (mInvert ? "!WordBound" : "WordBound") << std::endl;
  Instruction::toStream(os, indent);
}

const char* WordBound::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match " << (mInvert ? "not " : "") << "word bound... ";
#endif
  if (!mInvert)
  {
    // same in reverse mode?
    if (((cur <= info.beg) && CHAR_IS(*cur, WORD_CHAR)) ||
        ( CHAR_IS(*cur, WORD_CHAR) && !CHAR_IS(*(cur-1), WORD_CHAR)) ||
        (!CHAR_IS(*cur, WORD_CHAR) &&  CHAR_IS(*(cur-1), WORD_CHAR)))
    {
#ifdef _DEBUG
      std::cout << "OK (WordBound)" << std::endl;
#endif
      return matchRemain(cur, info);
    }
  }
  else
  {
    // same in reverse mode?
    if ((cur > info.beg) &&
        (cur+1 < info.end) &&
        (( CHAR_IS(*cur, WORD_CHAR) &&  CHAR_IS(*(cur-1), WORD_CHAR)) ||
         (!CHAR_IS(*cur, WORD_CHAR) && !CHAR_IS(*(cur-1), WORD_CHAR))))
    {
#ifdef _DEBUG
      std::cout << "OK (WordBound)" << std::endl;
#endif
      return matchRemain(cur, info);
    }
  }
#ifdef _DEBUG
  std::cout << "Failed" << std::endl;
#endif
  return 0;
}

// ---

LineStart::LineStart()
  : Instruction()
{
}

LineStart::~LineStart()
{
}

Instruction* LineStart::clone() const
{
  return new LineStart();
}

void LineStart::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << "LineStart" << std::endl;
  Instruction::toStream(os, indent);
}

const char* LineStart::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match line start... ";
#endif
  // NL: \r, \n or \r\n
  
  register char c0;
  
  if (cur <= info.beg)
  {
#ifdef _DEBUG
    std::cout << "OK (LineStart)" << std::endl;
#endif
    return matchRemain(cur, info);
  }
  else if (info.flags & Rex::Multiline)
  {
    c0 = *(cur-1);
    if (c0 == '\n' || c0 == '\r')
    {
#ifdef _DEBUG
      std::cout << "OK (LineStart)" << std::endl;
#endif
      return matchRemain(cur, info);
    }
  }
#ifdef _DEBUG
  std::cout << "Failed" << std::endl;
#endif
  return 0;
}

// ---

LineEnd::LineEnd()
  : Instruction()
{
}

LineEnd::~LineEnd()
{
}

Instruction* LineEnd::clone() const
{
  return new LineEnd();
}

void LineEnd::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << "LineEnd" << std::endl;
  Instruction::toStream(os, indent);
}

const char* LineEnd::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match line end... ";
#endif
  // NL: \r, \n or \r\n
  
  if (cur >= info.end)
  {
#ifdef _DEBUG
    std::cout << "OK (LineEnd)" << std::endl;
#endif
    return matchRemain(cur, info);
  }
  else if (info.flags & Rex::Multiline)
  {
    if (*cur == '\n' || *cur == '\r')
    {
#ifdef _DEBUG
      std::cout << "OK (LineEnd)" << std::endl;
#endif
      return matchRemain(cur, info);
    }
  }

#ifdef _DEBUG
  std::cout << "Failed" << std::endl;
#endif
  return 0;
}

// ---

StrStart::StrStart()
  : Instruction()
{
}

StrStart::~StrStart()
{
}

Instruction* StrStart::clone() const
{
  return new StrStart();
}

void StrStart::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << "StrStart" << std::endl;
  Instruction::toStream(os, indent);
}

const char* StrStart::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match string begin... ";
#endif
  if (cur <= info.beg) // cur == info.beg
  {
#ifdef _DEBUG
    std::cout << "OK (StrStart)" << std::endl;
#endif
    return matchRemain(cur, info);
  }
#ifdef _DEBUG
  std::cout << "Failed" << std::endl;
#endif
  return 0;
}

// ---

StrEnd::StrEnd()
  : Instruction()
{
}

StrEnd::~StrEnd()
{
}

Instruction* StrEnd::clone() const
{
  return new StrEnd();
}


void StrEnd::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << "StrEnd" << std::endl;
  Instruction::toStream(os, indent);
}

const char* StrEnd::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match string end... ";
#endif
  if ((cur >= info.end) || // c == e
      ((cur+1 == info.end) && ((*cur == '\n') || (*cur == '\r'))) ||
      ((cur+2 == info.end) && (*cur == '\r') && (*(cur+1) == '\n')))
  {
#ifdef _DEBUG
    std::cout << "OK (StrEnd)" << std::endl;
#endif
    return matchRemain(cur, info);
  }
#ifdef _DEBUG
  std::cout << "Failed" << std::endl;
#endif
  return 0;
}

// ---

BufferEnd::BufferEnd()
  : Instruction()
{
}

BufferEnd::~BufferEnd()
{
}

Instruction* BufferEnd::clone() const
{
  return new BufferEnd();
}

void BufferEnd::toStream(std::ostream &os, const std::string &indent) const
{
  os << indent << "BufferEnd" << std::endl;
  Instruction::toStream(os, indent);
}

const char* BufferEnd::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG
  std::cout << "Match buffer end... ";
#endif
  //if (!(info.flags & Rex::Reverse))
  //{
  if (cur == info.end)
  {
#ifdef _DEBUG
    std::cout << "OK (BufferEnd)" << std::endl;
#endif
    return matchRemain(cur, info);
  }
  //}
#ifdef _DEBUG
  std::cout << "Failed" << std::endl;
#endif
  return 0;
}

}

