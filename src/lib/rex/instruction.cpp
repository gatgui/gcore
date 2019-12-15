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

#include "instruction.h"
#include "charclass.h"
#include <gcore/rex.h>
#include <gcore/log.h>
#include <typeinfo>

namespace gcore
{

#ifdef _WIN32
# define strncasecmp _strnicmp
# define strcasecmp _stricmp
#endif

MatchInfo::MatchInfo()
   : beg(0)
   , end(0)
   , flags(0)
   , once(false)
{
}

MatchInfo::MatchInfo(const char *b, const char *e, unsigned short f, size_t ngroups)
   : beg(b)
   , end(e)
   , flags(f)
   , once(false)
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

MatchInfo::~MatchInfo()
{
   gmatch.clear();
   fstack.clear();
   cstack.clear();
   gclosed.clear();
   gnames.clear();
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
   : mNext(0)
   , mPrev(0)
   , mGroup(0)
{
}

Instruction::~Instruction()
{
   if (mNext)
   {
      delete mNext;
   }
}

void Instruction::toStream(std::ostream &os, const String &indent) const
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
      if (mGroup)
      {
         mNext->setGroup(mGroup);
      }
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
      //if (--cur < info.beg)
      cur = UTF8Prev(cur);
      if (cur < info.beg)
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
   if (!(info.flags & Rex::Reverse))
   {
      //++cur;
      cur = UTF8Next(cur);
   }
   
   return (info.once ? cur : matchRemain(cur, info));
}

const char* Instruction::matchRemain(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Instruction::matchRemain: (%s) \"%s\"", typeid(*this).name(), cur);
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

#ifdef _DEBUG_REX
   if (mGroup)
   {
      Log::PrintDebug("[gcore] rex/Instruction::matchRemain: Is in group %d", (int)mGroup->index());
   }
#endif
   
   if (mGroup && info.gclosed[mGroup] == false)
   {
#ifdef _DEBUG_REX
      Log::PrintDebug("[gcore] rex/Instruction::matchRemain: Group %d not closed yet", (int)mGroup->index());
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
#ifdef _DEBUG_REX
            Log::PrintDebug("[gcore] rex/Instruction::matchRemain: Re-open matched group %d", (int)mGroup->index());
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
   : Instruction()
   , mChar(c)
   , mUpperChar(c)
   , mLowerChar(c)
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
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Single::match: Match single character \'%c\' with \"%s\"... ", mChar, cur);
#endif
   
   register bool matched;
   
   if (preStep(cur, info))
   {
      matched = (info.flags & Rex::NoCase) ? (*cur == mLowerChar || *cur == mUpperChar) : (*cur == mChar);
      
      if (matched)
      {
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/Single::match: Succeeded");
#endif
         return postStep(cur, info);
      }
   }
   
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Single::match: Failed");
#endif
   return 0;
}

void Single::toStream(std::ostream &os, const String &indent) const
{
   os << indent << "Single \'" << mChar << "\'" << std::endl;
   Instruction::toStream(os, indent);
}

// ---

UnicodeSingle::UnicodeSingle(Codepoint c)
   : Instruction()
   , mCode(c)
   , mUpperChar('\0')
   , mLowerChar('\0')
{
   if (c < 128)
   {
      int diff = 'A' - 'a';
      char ch = (char) c;
      
      if (CHAR_IS(ch, LOWER_CHAR))
      {
         mLowerChar = ch;
         mUpperChar = (char)(ch + diff);
      }
      else if (CHAR_IS(ch, UPPER_CHAR))
      {
         mUpperChar = ch;
         mLowerChar = (char)(ch - diff);
      }
   }
}

UnicodeSingle::~UnicodeSingle()
{
}

Instruction* UnicodeSingle::clone() const
{
   return new UnicodeSingle(mCode);
}

const char* UnicodeSingle::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/UnicodeSingle::match: Match single character %x with \"%s\"... ", mCode, cur);
#endif
   
   register bool matched;
   
   if (preStep(cur, info))
   {
      if ((info.flags & Rex::NoCase) != 0 && IsUTF8SingleChar(*cur) && mLowerChar != '\0')
      {
         matched = (*cur == mLowerChar || *cur == mUpperChar);
      }
      else
      {
         matched = (DecodeUTF8(cur, cur - info.end) == mCode);
      }
      
      if (matched)
      {
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/UnicodeSingle::match: Succeeded");
#endif
         return postStep(cur, info);
      }
   }
   
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/UnicodeSingle::match: Failed");
#endif
   return 0;
}

void UnicodeSingle::toStream(std::ostream &os, const String &indent) const
{
   char tmp[16];
   size_t n = CodepointToASCII(mCode, ACF_VARIABLE, tmp, 16);
   tmp[n] = '\0';
   os << indent << "UnicodeSingle " << tmp << std::endl;
   Instruction::toStream(os, indent);
}

// ---

Any::Any()
   : Instruction()
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
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Any::match: Match any character...");
#endif
   
   if (preStep(cur, info))
   {
      if ((info.flags & Rex::DotMatchNewline) || (*cur != '\r' && *cur != '\n'))
      {
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/Any::match: Succeeded");
#endif
         return postStep(cur, info);
      }
   }

#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Any::match: Failed");
#endif
   
   return 0;
}

void Any::toStream(std::ostream &os, const String &indent) const
{
   os << indent << "Any" << std::endl;
   Instruction::toStream(os, indent);
}

// ---

Word::Word(bool invert)
   : Instruction()
   , mInvert(invert)
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
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Word::match: Match %sword character...", (mInvert ? "non-" : ""));
#endif
   if (preStep(cur, info))
   {
      if (mInvert ^ CHAR_IS(*cur, WORD_CHAR))
      {
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/Word::match: Succeeded");
#endif
         return postStep(cur, info);
      }
   }
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Word::match: Failed");
#endif
   return 0;
}

void Word::toStream(std::ostream &os, const String &indent) const
{
   os << indent << (mInvert ? "!Word" : "Word") << std::endl;
   Instruction::toStream(os, indent);
}

// ---


Digit::Digit(bool invert)
   : Instruction()
   , mInvert(invert)
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
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Digit::match: Match %sdigit character...", (mInvert ? "non-" : ""));
#endif
   if (preStep(cur, info))
   {
      if (mInvert ^ CHAR_IS(*cur, DIGIT_CHAR))
      {
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/Digit::match: Succeeded");
#endif
         return postStep(cur, info);
      }
   }
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Digit::match: Failed");
#endif
   return 0;
}

void Digit::toStream(std::ostream &os, const String &indent) const
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
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/LowerLetter::match: Match lower case letter...");
#endif
   if (preStep(cur, info))
   {
      if (CHAR_IS(*cur, LOWER_CHAR) || ((info.flags & Rex::NoCase) && CHAR_IS(*cur, UPPER_CHAR)))
      {
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/LowerLetter::match: Succeeded");
#endif
         return postStep(cur, info);
      }
   }
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/LowerLetter::match: Failed");
#endif
   return 0;
}

void LowerLetter::toStream(std::ostream &os, const String &indent) const
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
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/UpperLetter::match: Match upper case letter...");
#endif
   if (preStep(cur, info))
   {
      if (CHAR_IS(*cur, UPPER_CHAR) || ((info.flags & Rex::NoCase) && CHAR_IS(*cur, LOWER_CHAR)))
      {
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/UpperLetter::match: Succeeded");
#endif
         return postStep(cur, info);
      }
   }
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/UpperLetter::match: Failed");
#endif
   return 0;
}

void UpperLetter::toStream(std::ostream &os, const String &indent) const
{
   os << indent << "UpperLetter" << std::endl;
   Instruction::toStream(os, indent);
}

// ---

Letter::Letter(bool invert)
   : Instruction()
   , mInvert(invert)
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
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Letter::match: Match %sletter character...", (mInvert ? "non-" : ""));
#endif
   if (preStep(cur, info))
   {
      if (mInvert ^ CHAR_IS(*cur, LETTER_CHAR))
      {
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/Letter::match: Succeeded");
#endif
         return postStep(cur, info);
      }
   }
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Letter::match: Failed");
#endif
   return 0;
}

void Letter::toStream(std::ostream &os, const String &indent) const
{
   os << indent << (mInvert ? "!Letter" : "Letter") << std::endl;
   Instruction::toStream(os, indent);
}

// ---

Hexa::Hexa(bool invert)
   : Instruction()
   , mInvert(invert)
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
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Hexa::match: Match %shexadecimal number...", (mInvert ? "non-" : ""));
#endif
   if (preStep(cur, info))
   {
      if (mInvert ^ CHAR_IS(*cur, HEXA_CHAR))
      {
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/Hexa::match: Succeeded");
#endif
         return postStep(cur, info);
      }
   }
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Hexa::match: Failed");
#endif
   return 0;
}

void Hexa::toStream(std::ostream &os, const String &indent) const
{
   os << indent << (mInvert ? "!Hexa" : "Hexa") << std::endl;
   Instruction::toStream(os, indent);
}

// ---

Space::Space(bool invert)
   : Instruction()
   , mInvert(invert)
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
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Space::match: Match %sspace character...", (mInvert ? "non-" : ""));
#endif
   if (preStep(cur, info))
   {
      if (mInvert ^ CHAR_IS(*cur, SPACE_CHAR))
      {
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/Space::match: Succeeded");
#endif
         return postStep(cur, info);
      }
   }
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Space::match: Failed");
#endif
   return 0;
}

void Space::toStream(std::ostream &os, const String &indent) const
{
   os << indent << (mInvert ? "!Space" : "Space") << std::endl;
   Instruction::toStream(os, indent);
}

// ---

CharRange::CharRange(char from, char to)
   : Instruction()
   , mFrom(from)
   , mTo(to)
{
}

CharRange::~CharRange()
{
}

Instruction* CharRange::clone() const
{
   return new CharRange(mFrom, mTo);
}

void CharRange::toStream(std::ostream &os, const String &indent) const
{
   os << indent << "CharRange " << mFrom << "-" << mTo << std::endl;
   Instruction::toStream(os, indent);
}

const char* CharRange::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/CharRange::match: Match character in range [%c, %c]...", mFrom, mTo);
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
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/CharRange::match: Succeeded");
#endif
         return postStep(cur, info);
      }
   }
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/CharRange::match: Failed");
#endif
   return 0;
}

// ---

UnicodeCharRange::UnicodeCharRange(Codepoint from, Codepoint to)
   : Instruction()
   , mFrom(from)
   , mTo(to)
{
}

UnicodeCharRange::~UnicodeCharRange()
{
}

Instruction* UnicodeCharRange::clone() const
{
   return new UnicodeCharRange(mFrom, mTo);
}

void UnicodeCharRange::toStream(std::ostream &os, const String &indent) const
{
   char tmp[16];
   size_t n = 0;
   
   n = CodepointToASCII(mFrom, ACF_VARIABLE, tmp, 16);
   tmp[n] = '\0';
   
   os << indent << "UnicodeCharRange " << tmp << "-";
   
   n = CodepointToASCII(mTo, ACF_VARIABLE, tmp, 16);
   tmp[n] = '\0';
   
   os << tmp << std::endl;
   
   Instruction::toStream(os, indent);
}

const char* UnicodeCharRange::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/UnicodeCharRange::match: Match character in range [%x, %x]...", mFrom, mTo);
#endif
   register Codepoint c = InvalidCodepoint;
   register int casediff = 0;
   register bool matched = false;
   register char cc = '\0';
   
   if (preStep(cur, info))
   {
      if ((info.flags & Rex::NoCase) != 0 && IsUTF8SingleChar(*cur))
      {
         // ASCII character [0-127]
         casediff = 'A' - 'a';
         c = mFrom;
         while (c <= mTo)
         {
            if (c >= 128)
            {
               break;
            }
            cc = (char) c;
            if ((*cur == cc) ||
                (CHAR_IS(cc, LOWER_CHAR) && (*cur == cc + casediff)) ||
                (CHAR_IS(cc, UPPER_CHAR) && (*cur == cc - casediff)))
            {
               matched = true;
               break;
            }
            ++c;
         }
      }
      else
      {
         c = DecodeUTF8(cur, cur - info.end);
         matched = (mFrom <= c && c <= mTo);
      }
      
      if (matched)
      {
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/UnicodeCharRange::match: Succeeded");
#endif
         return postStep(cur, info);
      }
   }
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/UnicodeCharRange::match: Failed");
#endif
   return 0;
}

// ---


CharClass::CharClass(Instruction *klass, bool invert)
   : Instruction()
   , mInvert(invert)
   , mFirst(klass)
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

void CharClass::toStream(std::ostream &os, const String &indent) const
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
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/CharClass::match: Match character %sin class...", (mInvert ? "not " : ""));
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
#ifdef _DEBUG_REX
            Log::PrintDebug("[gcore] rex/CharClass::match: Succeeded");
#endif
            return postStep(cur, info);
         }
      }
#ifdef _DEBUG_REX
      Log::PrintDebug("[gcore] rex/CharClass::match: Failed");
#endif
      return 0;
   }
   else
   {
      //return rv; // only?
      if (rv != 0)
      {
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/CharClass::match: %s", (rv ? "Succeeded" : "Failed"));
#endif
         return matchRemain(rv, info);
      }
      return 0;
   }
}

// ---

Repeat::Repeat(Instruction *s, int min, int max, bool lazy)
   : Instruction()
   , mMin(min)
   , mMax(max)
   , mLazy(lazy)
   , mInst(s)
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

void Repeat::toStream(std::ostream &os, const String &indent) const
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
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Repeat::match: Match repeat...");
#endif
   register int count = 0;
   const char *rv = 0;
   
   if (!mInst)
   {
#ifdef _DEBUG_REX
      Log::PrintDebug("[gcore] rex/Repeat::match: Failed");
#endif
      return 0;
   }
   
   // match at leat mMin times (same for greedy and lazy)
   
   while (count < mMin)
   {
      rv = mInst->match(cur, info);
      if (!rv)
      {
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/Repeat::match: Failed");
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
#ifdef _DEBUG_REX
            Log::PrintDebug("[gcore] rex/Repeat::match: Failed");
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
#ifdef _DEBUG_REX
               Log::PrintDebug("[gcore] rex/Repeat::match: Repeat in group match remaining failed, re-open group %d", (int)mGroup->index());
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
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Repeat::match: %s", (rv ? "Succeeded" : "Failed"));
#endif
   return rv;
}

// ---


Alternative::Alternative(Instruction *i0, Instruction *i1)
   : Instruction()
   , mFirst(i0)
   , mSecond(i1)
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

void Alternative::toStream(std::ostream &os, const String &indent) const
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
             const String &name)
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
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Group::end: Restore flags %d -> %d (flags stack size = %d)", info.flags, info.fstack.back(), info.fstack.size());
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
#ifdef _DEBUG_REX
      Log::PrintDebug("[gcore] rex/Group::end: Failed");
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
#ifdef _DEBUG_REX
      Log::PrintDebug("[gcore] rex/Group::end: End capture %d of %d [%d, %d]", mIndex+1, info.gmatch.size(), info.gmatch[mIndex].first, info.gmatch[mIndex].second);
#endif
   }
   
   if (mZeroWidth)
   {
      cur = c;
   }
   /*
   if (mZeroWidth)
   {
#ifdef _DEBUG_REX
      Log::PrintDebug("[gcore] rex/Group::end: Zerowidth group, restore character pointer");
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

void Group::toStream(std::ostream &os, const String &indent) const
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
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Group::match [%d, '%s']", mIndex, mName.c_str());
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
   
#ifdef _DEBUG_REX
   Log::SetIndentLevel(Log::IndentLevel()+1);
   Log::PrintDebug("group: %d", mIndex);
   Log::PrintDebug("name: '%s'", mName.c_str());
   Log::PrintDebug("invert: %d", mInvert);
   Log::PrintDebug("zerowidth: %d", mZeroWidth);
   Log::PrintDebug("reverse: %d", ((mFlags & Rex::Reverse) == 1));
   Log::PrintDebug("nocase: %d", mNoCase);
   Log::PrintDebug("multiline: %d", mMultiline);
   Log::PrintDebug("dotmatchnewline: %d", mDotNewline);
   Log::SetIndentLevel(Log::IndentLevel()-1);
#endif
   
   if (mFirst)
   {
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
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/Group::match: Group not yet closed rv = 0x%x", (void*)rv);
#endif
         bool failed = (rv == 0);
         rv = failed ? cur : rv;
         if (end(failed, rv, info))
         {
#ifdef _DEBUG_REX
            Log::PrintDebug("[gcore] rex/Group::match: Succeeded (match remain)");
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
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/Group::match: %s", (rv == 0 ? "Failed" : "Succeeded"));
#endif
         return rv;
      }
   }
   else
   {
      // a group without instruction is used to modify flags for
      // the remaining of the expression
      info.flags = flags;
#ifdef _DEBUG_REX
      Log::PrintDebug("[gcore] rex/Group::match: Succeeded (flag modify only)");
#endif
      return matchRemain(cur, info);
   }
}

// ---

Backsubst::Backsubst(int index)
   : Instruction()
   , mIndex(index)
{      
}

Backsubst::Backsubst(const String &n)
   : Instruction()
   , mIndex(-1)
   , mName(n)
{
}

Backsubst::~Backsubst()
{
}

Instruction* Backsubst::clone() const
{
   return (mIndex > 0 ? new Backsubst(mIndex) : new Backsubst(mName));
}

void Backsubst::toStream(std::ostream &os, const String &indent) const
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
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Backsubst::match: Match backsubstitution %d/'%s'...", mIndex, mName.c_str());
#endif
   
   size_t index = 0;
   
   if (mIndex > 0)
   {
      index = mIndex;
   }
   else
   {
      std::map<String, size_t>::iterator it = info.gnames.find(mName);
      if (it == info.gnames.end() || it->second <= 0)
      {
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/Backsubst::match: Failed");
#endif
         return 0;
      }
      index = it->second;
   }
   
   if (index >= info.gmatch.size())
   {
#ifdef _DEBUG_REX
      Log::PrintDebug("[gcore] rex/Backsubst::match: Failed (invalid group index)");
#endif
      return 0;
   }
   
   if (info.gmatch[index].first < 0 || info.gmatch[index].second < 0)
   {
#ifdef _DEBUG_REX
      Log::PrintDebug("[gcore] rex/Backsubst::match: Failed (group not matched)");
#endif
      return 0;
   }
   
   size_t bslen = info.gmatch[index].second - info.gmatch[index].first;
   
   if (info.flags & Rex::Reverse)
   {
      size_t rlen = cur - info.beg;
      
      if (rlen < bslen)
      {
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/Backsubst::match: Failed");
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
#ifdef _DEBUG_REX
                  Log::PrintDebug("[gcore] rex/Backsubst::match: Failed");
#endif
                  return 0;
               }
               
               size_t i0 = CHAR_IS(*cur, LOWER_CHAR) ? *cur - 'a' : *cur - 'A';
               size_t i1 = CHAR_IS(*bs, LOWER_CHAR) ? *bs - 'a' : *bs - 'A';
               
               if (i0 != i1)
               {
#ifdef _DEBUG_REX
                  Log::PrintDebug("[gcore] rex/Backsubst::match: Failed");
#endif
                  return 0;
               }
            }
            else if (*cur != *bs)
            {
#ifdef _DEBUG_REX
               Log::PrintDebug("[gcore] rex/Backsubst::match: Failed");
#endif
               return 0;
            }
            --bs;
            --cur;
         }
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/Backsubst::match: Succeeded");
#endif
         return matchRemain(cur+1, info);
      }
      else
      {
         while (cur >= to)
         {
            if (*cur != *bs)
            {
#ifdef _DEBUG_REX
               Log::PrintDebug("[gcore] rex/Backsubst::match: Failed");
#endif
               return 0;
            }
            --bs;
            --cur;
         }
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/Backsubst::match: Succeeded");
#endif
         return matchRemain(cur+1, info);
      }
   }
   else
   {
      size_t rlen = info.end - cur;
      
      if (rlen < bslen)
      {
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/Backsubst::match: Failed");
#endif
         return 0;
      }
      
      if (info.flags & Rex::NoCase)
      {
         if (strncasecmp(cur, info.beg + info.gmatch[index].first, bslen) == 0)
         {
#ifdef _DEBUG_REX
            Log::PrintDebug("[gcore] rex/Backsubst::match: Succeeded");
#endif
            return matchRemain(cur + bslen, info);
         }
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/Backsubst::match: Failed");
#endif
         return 0;
      }
      else
      {
         if (strncmp(cur, info.beg + info.gmatch[index].first, bslen) == 0)
         {
#ifdef _DEBUG_REX
            Log::PrintDebug("[gcore] rex/Backsubst::match: Succeeded");
#endif
            return matchRemain(cur + bslen, info);
         }
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/Backsubst::match: Failed");
#endif
         return 0;
      }
   }

#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/Backsubst::match: Failed");
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

void WordStart::toStream(std::ostream &os, const String &indent) const
{
   os << indent << "WordStart" << std::endl;
   Instruction::toStream(os, indent);
}

const char* WordStart::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/WordStart::match: Match begining of word...");
#endif
   // same in reverse mode?
   if (CHAR_IS(*cur, WORD_CHAR) && ((cur <= info.beg) || !CHAR_IS(*(cur-1), WORD_CHAR)))
   {
#ifdef _DEBUG_REX
      Log::PrintDebug("[gcore] rex/WordStart::match: Succeeded");
#endif
      return matchRemain(cur, info);
   }
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/WordStart::match: Failed");
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

void WordEnd::toStream(std::ostream &os, const String &indent) const
{
   os << indent << "WordEnd" << std::endl;
   Instruction::toStream(os, indent);
}

const char* WordEnd::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/WordEnd::match: Match end of word...");
#endif
   // same in reverse mode?
   if (!CHAR_IS(*cur, WORD_CHAR) && ((cur <= info.beg) || CHAR_IS(*(cur-1), WORD_CHAR)))
   {
#ifdef _DEBUG_REX
      Log::PrintDebug("[gcore] rex/WordEnd::match: Succeeded");
#endif
      return matchRemain(cur, info);
   }
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/WordEnd::match: Failed");
#endif
   return 0;
}

// ---

WordBound::WordBound(bool invert)
   : Instruction()
   , mInvert(invert)
{
}

WordBound::~WordBound()
{
}

Instruction* WordBound::clone() const
{
   return new WordBound(mInvert);
}

void WordBound::toStream(std::ostream &os, const String &indent) const
{
   os << indent << (mInvert ? "!WordBound" : "WordBound") << std::endl;
   Instruction::toStream(os, indent);
}

const char* WordBound::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/WordBound::match: Match %sword boundaries...", (mInvert ? "non-" : ""));
#endif
   if (!mInvert)
   {
      // same in reverse mode?
      if (((cur <= info.beg) && CHAR_IS(*cur, WORD_CHAR)) ||
          ( CHAR_IS(*cur, WORD_CHAR) && !CHAR_IS(*(cur-1), WORD_CHAR)) ||
          (!CHAR_IS(*cur, WORD_CHAR) &&  CHAR_IS(*(cur-1), WORD_CHAR)))
      {
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/WordBOund::match: Succeeded");
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
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/WordBound::match: Succeeded");
#endif
         return matchRemain(cur, info);
      }
   }
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/WordBound::match: Failed");
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

void LineStart::toStream(std::ostream &os, const String &indent) const
{
   os << indent << "LineStart" << std::endl;
   Instruction::toStream(os, indent);
}

const char* LineStart::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/LineStart::match: Match begining of line...");
#endif
   // NL: \r, \n or \r\n
   
   register char c0;
   
   if (cur <= info.beg)
   {
#ifdef _DEBUG_REX
      Log::PrintDebug("[gcore] rex/LineStart::match: Succeeded");
#endif
      return matchRemain(cur, info);
   }
   else if (info.flags & Rex::Multiline)
   {
      c0 = *(cur-1);
      if (c0 == '\n' || c0 == '\r')
      {
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/LineStart::match: Succeeded");
#endif
         return matchRemain(cur, info);
      }
   }
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/LineStart::match: Failed");
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

void LineEnd::toStream(std::ostream &os, const String &indent) const
{
   os << indent << "LineEnd" << std::endl;
   Instruction::toStream(os, indent);
}

const char* LineEnd::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/LineEnd::match: Match line end...");
#endif
   // NL: \r, \n or \r\n
   
   if (cur >= info.end)
   {
#ifdef _DEBUG_REX
      Log::PrintDebug("[gcore] rex/LineEnd::match: Succeeded");
#endif
      return matchRemain(cur, info);
   }
   else if (info.flags & Rex::Multiline)
   {
      if (*cur == '\n' || *cur == '\r')
      {
#ifdef _DEBUG_REX
         Log::PrintDebug("[gcore] rex/LineEnd::match: Succeeded");
#endif
         return matchRemain(cur, info);
      }
   }

#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/LineEnd::match: Failed");
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

void StrStart::toStream(std::ostream &os, const String &indent) const
{
   os << indent << "StrStart" << std::endl;
   Instruction::toStream(os, indent);
}

const char* StrStart::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/StrStart::match: Match begining of string...");
#endif
   if (cur <= info.beg) // cur == info.beg
   {
#ifdef _DEBUG_REX
      Log::PrintDebug("[gcore] rex/StrStart::match: Succeeded");
#endif
      return matchRemain(cur, info);
   }
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/StrStart::match: Failed");
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


void StrEnd::toStream(std::ostream &os, const String &indent) const
{
   os << indent << "StrEnd" << std::endl;
   Instruction::toStream(os, indent);
}

const char* StrEnd::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/StrEnd::match: Match end of string...");
#endif
   if ((cur >= info.end) || // c == e
       ((cur+1 == info.end) && ((*cur == '\n') || (*cur == '\r'))) ||
       ((cur+2 == info.end) && (*cur == '\r') && (*(cur+1) == '\n')))
   {
#ifdef _DEBUG_REX
      Log::PrintDebug("[gcore] rex/StrEnd::match: Succeeded");
#endif
      return matchRemain(cur, info);
   }
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/StrEnd::match: Failed");
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

void BufferEnd::toStream(std::ostream &os, const String &indent) const
{
   os << indent << "BufferEnd" << std::endl;
   Instruction::toStream(os, indent);
}

const char* BufferEnd::match(const char *cur, MatchInfo &info) const
{
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/BufferEnd::match: Match end of buffer...");
#endif
   //if (!(info.flags & Rex::Reverse))
   //{
   if (cur == info.end)
   {
#ifdef _DEBUG_REX
      Log::PrintDebug("[gcore] rex/BufferEnd::match: Succeeded");
#endif
      return matchRemain(cur, info);
   }
   //}
#ifdef _DEBUG_REX
   Log::PrintDebug("[gcore] rex/BufferEnd::match: Failed");
#endif
   return 0;
}

// ---

Conditional::Conditional(int index, Instruction *ifTrue, Instruction *ifFalse)
   : mIndex(index)
   , mTrue(ifTrue)
   , mFalse(ifFalse)
{
}

Conditional::Conditional(const String &n, Instruction *ifTrue, Instruction *ifFalse)
   : mIndex(-1)
   , mName(n)
   , mTrue(ifTrue)
   , mFalse(ifFalse)
{
}

Conditional::~Conditional()
{
   if (mTrue)
   {
      delete mTrue;
   }
   if (mFalse)
   {
      delete mFalse;
   }
}

Instruction* Conditional::clone() const
{
   if (mIndex > 0)
   {
      return new Conditional(mIndex, (mTrue ? mTrue->clone() : 0), (mFalse ? mFalse->clone() : 0));
   }
   else
   {
      return new Conditional(mName, (mTrue ? mTrue->clone() : 0), (mFalse ? mFalse->clone() : 0));
   }
}

void Conditional::toStream(std::ostream &os, const String &indent) const
{
   if (mTrue)
   {
      os << indent << "If group ";
      if (mIndex > 0)
      {
         os << mIndex;
      }
      else
      {
         os << mName;
      }
      os << " captured" << std::endl;
      mTrue->toStream(os, indent+"  ");
      if (mFalse)
      {
         os << indent << "Else" << std::endl;
         mFalse->toStream(os, indent+"  ");
      }
   }
   Instruction::toStream(os, indent);
}

void Conditional::setGroup(class Group *grp)
{
   Instruction::setGroup(grp);
   if (mTrue)
   {
      mTrue->setGroup(grp);
   }
   if (mFalse)
   {
      mFalse->setGroup(grp);
   }
}

const char* Conditional::match(const char *cur, MatchInfo &info) const
{
   if (!mTrue)
   {
      return matchRemain(cur, info);
   }
   size_t index = 0;
   if (mName.length() > 0)
   {
      std::map<String, size_t>::iterator it = info.gnames.find(mName);
      if (it != info.gnames.end())
      {
         index = it->second;
      }
   }
   else
   {
      index = mIndex;
   }
   const char *rv = cur;
   if (index == 0 ||
       index >= info.gmatch.size() ||
       info.gmatch[mIndex].first == -1 ||
       info.gmatch[mIndex].second == -1)
   {
      if (mFalse)
      {
         rv = mFalse->match(cur, info);
      }
   }
   else
   {
      rv = mTrue->match(cur, info);
   }
   if (rv != 0)
   {
      return matchRemain(rv, info);
   }
   else
   {
      return rv;
   }
}

} // gcore

