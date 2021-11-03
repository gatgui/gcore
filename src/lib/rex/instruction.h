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

#ifndef __gcore_rex_instruction_h_
#define __gcore_rex_instruction_h_

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <string>

namespace gcore {

struct MatchInfo
{
  const char *beg;
  const char *end;
  unsigned short flags;
  std::vector<std::pair<int,int> > gmatch;
  std::vector<unsigned short> fstack;
  std::vector<const char*> cstack;
  bool once;
  std::map<const class Group*, bool> gclosed;
  std::map<std::string, size_t> gnames;
  
  MatchInfo();
  MatchInfo(const char *b, const char *e, unsigned short flags, size_t ngroups);
  MatchInfo(const MatchInfo &rhs);
  
  MatchInfo& operator=(const MatchInfo &rhs);
};

class Instruction
{
  public:
    
    Instruction();
    virtual ~Instruction();
    
    virtual void setGroup(class Group *grp);
    void setNext(Instruction *inst);
    void setPrev(Instruction *inst);
    
    class Group* group() const;
    Instruction* next() const;
    Instruction* prev() const;
    
    virtual Instruction* clone() const = 0;
    
    /**
      * match input string against this instruction
      * @param input Imput character
      * @param flags Execution flags
      * @see Rex::Flags
      * @param groups Matched group ranges
      * @return NULL if instruction didn't match, pointer to next character on success
      */
    virtual const char* match(const char *cur, MatchInfo &info) const = 0;
    
    virtual void toStream(std::ostream &os, const std::string &indent="") const = 0;
    
    static Instruction* CloneList(Instruction *i);
    
  protected:
    
    bool preStep(const char *&cur, MatchInfo &info) const;
    const char* postStep(const char *cur, MatchInfo &info) const;
    virtual const char* matchRemain(const char *cur, MatchInfo &info) const;
    
  protected:
    
    Instruction *mNext;
    Instruction *mPrev;
    class Group *mGroup;
};

class Single : public Instruction
{
  public:
    
    Single(char c);
    virtual ~Single();
    
    virtual Instruction* clone() const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    
  protected:
    
    char mChar;
    char mUpperChar;
    char mLowerChar;
};

class Any : public Instruction
{
  public:
    
    Any();
    virtual ~Any();
    
    virtual Instruction* clone() const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
};

class Word : public Instruction
{
  public:
    
    Word(bool invert);
    virtual ~Word();
    
    virtual Instruction* clone() const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    
  protected:
    
    bool mInvert;
};

class Digit : public Instruction
{
  public:
    
    Digit(bool invert);
    virtual ~Digit();
    
    virtual Instruction* clone() const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    
  protected:
    
    bool mInvert;
};

class LowerLetter : public Instruction
{
  public:
    
    LowerLetter();
    virtual ~LowerLetter();
      
    virtual Instruction* clone() const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
};

class UpperLetter : public Instruction
{
  public:
    
    UpperLetter();
    virtual ~UpperLetter();
    
    virtual Instruction* clone() const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
};

class Letter : public Instruction
{
  public:
    
    Letter(bool invert);
    virtual ~Letter();
    
    virtual Instruction* clone() const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    
  protected:
    
    bool mInvert;
};

class Hexa : public Instruction
{
  public:
    
    Hexa(bool invert);
    virtual ~Hexa();
    
    virtual Instruction* clone() const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    
  protected:
    
    bool mInvert;
};

class Space : public Instruction
{
  public:
    
    Space(bool invert);
    virtual ~Space();
    
    virtual Instruction* clone() const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    
  protected:
    
    bool mInvert;
};

class CharRange : public Instruction
{
  public:
    
    CharRange(char from, char to);
    virtual ~CharRange();
    
    virtual Instruction* clone() const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
    
  protected:
    
    char mFrom;
    char mTo;
};

class CharClass : public Instruction
{
  public:
    
    CharClass(Instruction *klass, bool invert);
    virtual ~CharClass();
    
    virtual Instruction* clone() const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
    
  protected:
    
    bool mInvert;
    Instruction *mFirst;
};

class Repeat : public Instruction
{
  public:
    
    Repeat(Instruction *s, int min=0, int max=-1, bool lazy=false);
    virtual ~Repeat();
    
    void setInstruction(Instruction *i);
    
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    virtual Instruction* clone() const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
    
  protected:
    
    int mMin;
    int mMax;
    bool mLazy;
    Instruction *mInst;
};

class Alternative : public Instruction
{
  public:
    
    Alternative(Instruction *i0, Instruction *i1);
    virtual ~Alternative();
    
    virtual Instruction* clone() const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
    
    virtual void setGroup(class Group *grp);
    
    inline Instruction* first() const {return mFirst;}
    inline Instruction* second() const {return mSecond;}
    
  protected:
    
    Instruction *mFirst;
    Instruction *mSecond;
};

class Group : public Instruction
{
  public:
    
    enum TriState
    {
      Off = 0,
      On,
      Inherit
    };
    
    Group(int index, Instruction *fisrt, bool zerowidth, bool invert,
          unsigned short flags, TriState nc, TriState ml, TriState dnl,
          const std::string &name="");
    virtual ~Group();
    
    bool end(bool failure, const char *&cur, MatchInfo &info) const;
    void open(const char *cur, MatchInfo &info) const;
    
    inline bool empty() const {return mFirst == 0;}
    inline bool zeroWidth() const {return mZeroWidth;}
    inline const std::string& name() const {return mName;}
    inline size_t index() const {return mIndex;}
    
    virtual Instruction* clone() const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
    
  protected:
    
    int mIndex;
    Instruction *mFirst;
    Instruction *mLast;
    bool mZeroWidth;
    bool mInvert;
    TriState mDotNewline;
    TriState mNoCase;
    TriState mMultiline;
    unsigned short mFlags;
    std::string mName;
};

class Backsubst : public Instruction
{
  public:
    
    Backsubst(int index);
    Backsubst(const std::string &n);
    virtual ~Backsubst();
    
    virtual Instruction* clone() const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
    
  protected:
    
    int mIndex;
    std::string mName;
};

class WordStart : public Instruction
{
  public:
    
    WordStart();
    virtual ~WordStart();
    
    virtual Instruction* clone() const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
};

class WordEnd : public Instruction
{
  public:
  
    WordEnd();
    virtual ~WordEnd();
    
    virtual Instruction* clone() const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
};

class WordBound : public Instruction
{
  public:
  
    WordBound(bool invert);
    virtual ~WordBound();
    
    virtual Instruction* clone() const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
  
  protected:
    
    bool mInvert;
};

class LineStart : public Instruction
{
  public:
  
    LineStart();
    virtual ~LineStart();
    
    virtual Instruction* clone() const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
};

class LineEnd : public Instruction
{
  public:
  
    LineEnd();
    virtual ~LineEnd();
    
    virtual Instruction* clone() const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
};

class StrStart : public Instruction
{
  public:
  
    StrStart();
    virtual ~StrStart();
    
    virtual Instruction* clone() const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
};

// End of string
// \Z hum should match before \n if trailing one
class StrEnd : public Instruction
{
  public:
  
    StrEnd();
    virtual ~StrEnd();
    
    virtual Instruction* clone() const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
};

// End of string
// \z
class BufferEnd : public Instruction
{
  public:
  
    BufferEnd();
    virtual ~BufferEnd();
    
    virtual Instruction* clone() const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
};

// Conditional
// (?(id|name)true|false)
class Conditional : public Instruction
{
  public:
    
    Conditional(int index, Instruction *ifTrue, Instruction *ifFalse=0);
    Conditional(const std::string &n, Instruction *ifTrue, Instruction *ifFalse=0);
    virtual ~Conditional();
    
    virtual Instruction* clone() const;
    virtual void toStream(std::ostream &os, const std::string &indent="") const;
    virtual const char* match(const char *cur, MatchInfo &info) const;
    
    virtual void setGroup(class Group *grp);
  
  protected:
    
    int mIndex;
    std::string mName;
    Instruction *mTrue;
    Instruction *mFalse;
};

}

#endif
