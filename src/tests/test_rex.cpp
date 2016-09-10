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

#include <gcore/rex.h>
#include <iostream>
#include <string>
#include <vector>
#include <set>

using namespace gcore;

class UnitTest
{
  public:
    
    UnitTest(const std::string &name)
      : mName(name)
    {
    }
    
    virtual ~UnitTest()
    {
    }
    
    virtual bool execute(bool verbose=false) = 0;
    
    const std::string& name() const
    {
      return mName;
    }
    
    void setName(const std::string &n)
    {
      mName = n;
    }
    
  protected:
    
    std::string mName;
};

class TestSuite
{
  public:
    
    TestSuite()
    {
    }
    
    ~TestSuite()
    {
      for (size_t i=0; i<mTests.size(); ++i)
      {
        delete mTests[i];
      }
    }
    
    void addTest(UnitTest *test)
    {
      mTests.push_back(test);
    }
    
    size_t numTests() const
    {
      return mTests.size();
    }
    
    UnitTest* getTest(size_t idx) const
    {
      if (idx < mTests.size())
      {
        return mTests[idx];
      }
      else
      {
        return 0;
      }
    }
    
    void execute(bool verbose=false)
    {
      size_t nsuccess = 0;
      std::vector<std::string> failedTests;
      
      for (size_t i=0; i<mTests.size(); ++i)
      {
        std::cout << "=== Executing test " << (i+1) << " out of " << mTests.size() << ": " << mTests[i]->name() << std::endl;
        if (mTests[i]->execute(verbose))
        {
          nsuccess += 1;
          std::cout << "===   Success" << std::endl;
        }
        else
        {
          failedTests.push_back(mTests[i]->name());
          std::cout << "===   Failure" << std::endl;
        }
      }
      
      std::cout << std::endl;
      std::cout << nsuccess << " test(s) succeeded." << std::endl;
      std::cout << (mTests.size() - nsuccess) << " test(s) failed." << std::endl;
      for (size_t i=0; i<failedTests.size(); ++i)
      {
        std::cout << "  " << failedTests[i] << std::endl;
      }
    }
  
  protected:
    
    std::vector<UnitTest*> mTests;
};

// ---

class RexTest : public UnitTest
{
  public:
    
    RexTest(const std::string &name,
            const std::string &exp,
            const std::string &str,
            bool expectedResult,
            size_t ngroups=0,
            const char **expectedGroups=0,
            unsigned short flags=0,
            size_t offset=0,
            size_t len=size_t(-1))
      : UnitTest(name)
      , mExp(exp)
      , mStr(str)
      , mExpectedResult(expectedResult)
      , mGroups(expectedGroups)
      , mNumGroups(ngroups)
      , mFlags(flags)
      , mOffset(offset)
      , mLength(len)
    {
      
    }
    
    virtual ~RexTest()
    {
    }
    
    void reverse()
    {
      if (mFlags & Rex::Reverse)
      {
        mFlags = mFlags & ~Rex::Reverse;
      }
      else
      {
        mFlags = mFlags | Rex::Reverse;
      }
    }
    
    virtual bool execute(bool verbose=false)
    {
      RexMatch match;
      
      if (verbose)
      {
        std::cout << "Code: " << std::endl << mExp << std::endl;
      }
      
      bool rv =  mExp.search(mStr, match, mFlags, mOffset, mLength);
      
      if (rv != mExpectedResult)
      {
        if (verbose)
        {
          std::cerr << "*** Expression \"" << mExp.get() << "\" should "
                    << (mExpectedResult ? "" : "not") << " match \""
                    << mStr << "\"" << std::endl;
        }
        return false;
      }
      
      if (rv == true)
      {
        for (size_t i=0; i<mNumGroups; ++i)
        {
          if (match.group(i) != mGroups[i])
          {
            std::cerr << "*** Got \"" << match.group(i) << "\" for group "
                      << i << ". Expected \"" << mGroups[i]
                      << "\"" << std::endl;
            return false;
          }
        }
      }
      
      return true;
    }
    
  protected:
    
    Rex mExp;
    std::string mStr;
    bool mExpectedResult;
    const char ** mGroups;
    size_t mNumGroups;
    unsigned short mFlags;
    size_t mOffset;
    size_t mLength;
};

// ---

inline bool doTest(const std::string &n, const std::set<std::string> &list)
{
  return (list.size() == 0 || list.find(n) != list.end());
}

int main(int argc, char **argv)
{
  TestSuite suite;
  
  std::set<std::string> tests;
  for (int i=1; i<argc; ++i)
  {
    tests.insert(argv[i]);
  }
  
  const char *results0[] = {"hello"};
  const char *results1[] = {"frel"};
  const char *results2[] = {"013"};
  const char *results3[] = {"oo01"};
  const char *results4[] = {" 98"};
  const char *results5[] = {"o013"};
  const char *results6[] = {"Bal"};
  const char *results7[] = {"poo0"};
  const char *results8[] = {"[0"};
  const char *results9[] = {"[-"};
  const char *results10[] = {"[]"};
  const char *results11[] = {"aaaaaab"};
  const char *results12[] = {"aaaab"};
  //const char *results13[] = {"1,2,3,4,5,6,7,8,9,10,11,P", "11,"};
  const char *results14[] = {"really hello"};
  const char *results15[] = {"\nreally hello"};
  const char *results16[] = {"aaaaaa"};
  const char *results17[] = {"aaab"};
  const char *results18[] = {"hello"};
  const char *results19[] = {"987"};
  const char *results20[] = {"aaab"};
  const char *results21[] = {"hello", "hello"};
  const char *results22[] = {"10 worlds", "worlds"};
  const char *results23[] = {"edo"};
  const char *results24[] = {"alb"};
  const char *results25[] = {"EDO"};
  const char *results26[] = {"hello hello", "hello"};
  const char *results27[] = {"HellO hELLo", "HellO"};
  const char *results28[] = {"HellO hELLo", "hELLo"};
  
  if (doTest("Characters", tests)) suite.addTest(new RexTest("Characters", RAW("hello"), "This is an hello world", true, 1, results0));
  if (doTest("Character class 0", tests)) suite.addTest(new RexTest("Character class 0", RAW("fre[ld]"), "frelon", true, 1, results1));
  if (doTest("Character class 1", tests)) suite.addTest(new RexTest("Character class 1", RAW("fre[a-z]"), "frelon", true, 1, results1));
  if (doTest("Character class 2", tests)) suite.addTest(new RexTest("Character class 2", RAW("fre[^a-d]"), "frelon", true, 1, results1));
  if (doTest("Character class 3", tests)) suite.addTest(new RexTest("Character class 3", RAW("fre[^a-d]"), "fredonner", false, 0, 0));
  if (doTest("Character class 4", tests)) suite.addTest(new RexTest("Character class 4", RAW("\[[]-0-9]"), "array[087]", true, 1, results8));
  if (doTest("Character class 5", tests)) suite.addTest(new RexTest("Character class 5", RAW("\[[]-0-9]"), "array[-087]", true, 1, results9));
  if (doTest("Character class 6", tests)) suite.addTest(new RexTest("Character class 6", RAW("\[[]-0-9]"), "array[]", true, 1, results10));
  if (doTest("Predefined class 0", tests)) suite.addTest(new RexTest("Predefined class 0", RAW("\d\d\d"), " poo013 987", true, 1, results2));
  if (doTest("Predefined class 1", tests)) suite.addTest(new RexTest("Predefined class 1", RAW("\w\w\d\d"), " poo013 987", true, 1, results3));
  if (doTest("Predefined class 2", tests)) suite.addTest(new RexTest("Predefined class 2", RAW("\W\d\d"), " poo013 987", true, 1, results2));
  if (doTest("Predefined class 3", tests)) suite.addTest(new RexTest("Predefined class 3", RAW("\S\d\d\d"), " poo013 987", true, 1, results5));
  if (doTest("Predefined class 4", tests)) suite.addTest(new RexTest("Predefined class 4", RAW("\s\d\d"), " poo013 987", true, 1, results4));
  if (doTest("Predefined class 5", tests)) suite.addTest(new RexTest("Predefined class 5", RAW("\w\D\d\d"), " poo013 987", true, 1, results3));
  if (doTest("Predefined class 6", tests)) suite.addTest(new RexTest("Predefined class 6", RAW("\u\U\U"), " BAl ball Ball", true, 1, results6));
  if (doTest("Predefined class 7", tests)) suite.addTest(new RexTest("Predefined class 7", RAW("\l\l\l\L"), " poo013 987", true, 1, results7));  
  if (doTest("Repeat 1", tests)) suite.addTest(new RexTest("Repeat 0", RAW("a*aaab"), " aaaaaab", true, 1, results11));
  if (doTest("Repeat 2", tests)) suite.addTest(new RexTest("Repeat 1", RAW("a*?aaab"), " aaaaaab", true, 1, results11));
  if (doTest("Repeat 3", tests)) suite.addTest(new RexTest("Repeat 2", RAW("a{,3}ab"), " aaaaaab", true, 1, results12));
  if (doTest("Repeat 4", tests)) suite.addTest(new RexTest("Repeat 3", RAW("a{6,}ab"), " aaaaaab", false, 0, 0));
  if (doTest("Zerowidth 0", tests)) suite.addTest(new RexTest("Zerowidth 0", RAW("^.*hello"), "really hello", true, 1, results14));
  if (doTest("Zerowidth 1", tests)) suite.addTest(new RexTest("Zerowidth 1", RAW("$.*hello"), "\nreally hello", true, 1, results15, Rex::Multiline|Rex::DotMatchNewline));
  if (doTest("Zerowidth 2", tests)) suite.addTest(new RexTest("Zerowidth 2", RAW("\<a.*a\>"), " aaaaaa ", true, 1, results16));
  if (doTest("Zerowidth 3", tests)) suite.addTest(new RexTest("Zerowidth 3", RAW("\ba.*b\B"), " aaabaa ", true, 1, results17));
  if (doTest("Zerowidth 4", tests)) suite.addTest(new RexTest("Zerowidth 4", RAW("\Ahello\Z"), "hello\nhello", false, 0, 0));
  if (doTest("Zerowidth 5", tests)) suite.addTest(new RexTest("Zerowidth 5", RAW("^hello$"), "goodbye\nhello", true, 1, results18, Rex::Multiline));
  if (doTest("Zerowidth 6", tests)) suite.addTest(new RexTest("Zerowidth 6", RAW("\Ahello\Z"), "hello\n", true, 1, results18)); // \Z will match before trailing newline if any
  if (doTest("Zerowidth 7", tests)) suite.addTest(new RexTest("Zerowidth 7", RAW("\Ahello\z"), "hello\n", false, 0, 0));
  if (doTest("Zerowidth 8", tests)) suite.addTest(new RexTest("Zerowidth 8", RAW("\Ahello\z"), "hello", true, 1, results18));
  
  if (doTest("[Reverse] Characters", tests)) suite.addTest(new RexTest("[Reverse] Characters", RAW("hello"), "This is an hello world", true, 1, results0, Rex::Reverse));
  if (doTest("[Reverse] Character class 0", tests)) suite.addTest(new RexTest("[Reverse] Character class 0", RAW("fre[ld]"), "frelon", true, 1, results1, Rex::Reverse));
  if (doTest("[Reverse] Character class 1", tests)) suite.addTest(new RexTest("[Reverse] Character class 1", RAW("fre[a-z]"), "frelon", true, 1, results1, Rex::Reverse));
  if (doTest("[Reverse] Character class 2", tests)) suite.addTest(new RexTest("[Reverse] Character class 2", RAW("fre[^a-d]"), "frelon", true, 1, results1, Rex::Reverse));
  if (doTest("[Reverse] Character class 3", tests)) suite.addTest(new RexTest("[Reverse] Character class 3", RAW("fre[^a-d]"), "fredonner", false, 0, 0, Rex::Reverse));
  if (doTest("[Reverse] Character class 4", tests)) suite.addTest(new RexTest("[Reverse] Character class 4", RAW("\[[]-0-9]"), "array[087]", true, 1, results8, Rex::Reverse));
  if (doTest("[Reverse] Character class 5", tests)) suite.addTest(new RexTest("[Reverse] Character class 5", RAW("\[[]-0-9]"), "array[-087]", true, 1, results9, Rex::Reverse));
  if (doTest("[Reverse] Character class 6", tests)) suite.addTest(new RexTest("[Reverse] Character class 6", RAW("\[[]-0-9]"), "array[]", true, 1, results10, Rex::Reverse));
  if (doTest("[Reverse] Predefined class 0", tests)) suite.addTest(new RexTest("[Reverse] Predefined class 0", RAW("\d\d\d"), " poo013 987", true, 1, results19, Rex::Reverse));
  if (doTest("[Reverse] Predefined class 1", tests)) suite.addTest(new RexTest("[Reverse] Predefined class 1", RAW("\w\w\d\d"), " poo013 987", true, 1, results3, Rex::Reverse));
  if (doTest("[Reverse] Predefined class 2", tests)) suite.addTest(new RexTest("[Reverse] Predefined class 2", RAW("\W\d\d"), " poo013 987", true, 1, results19, Rex::Reverse));
  if (doTest("[Reverse] Predefined class 3", tests)) suite.addTest(new RexTest("[Reverse] Predefined class 3", RAW("\S\d\d\d"), " poo013 987", true, 1, results5, Rex::Reverse));
  if (doTest("[Reverse] Predefined class 4", tests)) suite.addTest(new RexTest("[Reverse] Predefined class 4", RAW("\s\d\d"), " poo013 987", true, 1, results4, Rex::Reverse));
  if (doTest("[Reverse] Predefined class 5", tests)) suite.addTest(new RexTest("[Reverse] Predefined class 5", RAW("\w\D\d\d"), " poo013 987", true, 1, results3, Rex::Reverse));
  if (doTest("[Reverse] Predefined class 6", tests)) suite.addTest(new RexTest("[Reverse] Predefined class 6", RAW("\u\U\U"), " BAl ball Ball", true, 1, results6, Rex::Reverse));
  if (doTest("[Reverse] Predefined class 7", tests)) suite.addTest(new RexTest("[Reverse] Predefined class 7", RAW("\l\l\l\L"), " poo013 987", true, 1, results7, Rex::Reverse));  
  if (doTest("[Reverse] Repeat 0", tests)) suite.addTest(new RexTest("[Reverse] Repeat 0", RAW("a*aaab"), " aaaaaab", true, 1, results11, Rex::Reverse));
  if (doTest("[Reverse] Repeat 1", tests)) suite.addTest(new RexTest("[Reverse] Repeat 1", RAW("a*?aaab"), " aaaaaab", true, 1, results20, Rex::Reverse));
  if (doTest("[Reverse] Repeat 2", tests)) suite.addTest(new RexTest("[Reverse] Repeat 2", RAW("a{,3}ab"), " aaaaaab", true, 1, results12, Rex::Reverse));
  if (doTest("[Reverse] Repeat 3", tests)) suite.addTest(new RexTest("[Reverse] Repeat 3", RAW("a{6,}ab"), " aaaaaab", false, 0, 0, Rex::Reverse));
  if (doTest("[Reverse] Zerowidth 0", tests)) suite.addTest(new RexTest("[Reverse] Zerowidth 0", RAW("^.*hello"), "really hello", true, 1, results14, Rex::Reverse));
  if (doTest("[Reverse] Zerowidth 1", tests)) suite.addTest(new RexTest("[Reverse] Zerowidth 1", RAW("$.*hello"), "\nreally hello", true, 1, results15, Rex::Reverse|Rex::Multiline|Rex::DotMatchNewline));
  if (doTest("[Reverse] Zerowidth 2", tests)) suite.addTest(new RexTest("[Reverse] Zerowidth 2", RAW("\<a.*a\>"), " aaaaaa ", true, 1, results16, Rex::Reverse));
  if (doTest("[Reverse] Zerowidth 3", tests)) suite.addTest(new RexTest("[Reverse] Zerowidth 3", RAW("\ba.*b\B"), " aaabaa ", true, 1, results17, Rex::Reverse));
  if (doTest("[Reverse] Zerowidth 4", tests)) suite.addTest(new RexTest("[Reverse] Zerowidth 4", RAW("\Ahello\Z"), "hello\nhello", false, 0, 0, Rex::Reverse));
  if (doTest("[Reverse] Zerowidth 5", tests)) suite.addTest(new RexTest("[Reverse] Zerowidth 5", RAW("^hello$"), "goodbye\nhello", true, 1, results18, Rex::Reverse|Rex::Multiline));
  if (doTest("[Reverse] Zerowidth 6", tests)) suite.addTest(new RexTest("[Reverse] Zerowidth 6", RAW("\Ahello\Z"), "hello\n", true, 1, results18, Rex::Reverse)); // \Z will match before trailing newline if any
  if (doTest("[Reverse] Zerowidth 7", tests)) suite.addTest(new RexTest("[Reverse] Zerowidth 7", RAW("\Ahello\z"), "hello\n", false, 0, 0, Rex::Reverse));
  if (doTest("[Reverse] Zerowidth 8", tests)) suite.addTest(new RexTest("[Reverse] Zerowidth 8", RAW("\Ahello\z"), "hello", true, 1, results18, Rex::Reverse));
  
  if (doTest("Groups 0", tests)) suite.addTest(new RexTest("Groups 0", RAW("(hello)"), "hello world", true, 2, results21));
  if (doTest("Groups 1", tests)) suite.addTest(new RexTest("Groups 1", RAW("\d*\s+(\w+)"), "10 worlds", true, 2, results22));
  if (doTest("Groups 2", tests)) suite.addTest(new RexTest("Groups 2", RAW("(?!alb)edo"), "torpedo", true, 1, results23));
  if (doTest("Groups 3", tests)) suite.addTest(new RexTest("Groups 3", RAW("(?!alb)edo"), "albedo", true, 1, results23));
  if (doTest("Groups 4", tests)) suite.addTest(new RexTest("Groups 4", RAW("(?<!alb)edo"), "albedo", false, 0, 0));
  if (doTest("Groups 5", tests)) suite.addTest(new RexTest("Groups 5", RAW("(alb(?!edo)"), "albedo", false, 0, 0));
  if (doTest("Groups 6", tests)) suite.addTest(new RexTest("Groups 6", RAW("alb(?=edo)"), "albedo", true, 1, results24));
  if (doTest("Groups 7", tests)) suite.addTest(new RexTest("Groups 7", RAW("(?<=alb)edo"), "albedo", true, 1, results23));
  if (doTest("Groups 8", tests)) suite.addTest(new RexTest("Groups 8", RAW("(?i)edo"), "ALBEDO", true, 1, results25));
  if (doTest("Groups 9", tests)) suite.addTest(new RexTest("Groups 9", RAW("(\w+)\s+\1"), "hello hello", true, 2, results26));
  if (doTest("Groups 10", tests)) suite.addTest(new RexTest("Groups 10", RAW("(\w+)\s+\1"), "HellO hELLo", true, 2, results27, Rex::NoCase));
  
  if (doTest("[Reverse] Groups 0", tests)) suite.addTest(new RexTest("[Reverse] Groups 0", RAW("(hello)"), "hello world", true, 2, results21, Rex::Reverse));
  if (doTest("[Reverse] Groups 1", tests)) suite.addTest(new RexTest("[Reverse] Groups 1", RAW("\d*\s+(\w+)"), "10 worlds", true, 2, results22, Rex::Reverse));
  if (doTest("[Reverse] Groups 9", tests)) suite.addTest(new RexTest("[Reverse] Groups 9", RAW("\1\s+(\w+)"), "hello hello", true, 2, results26, Rex::Reverse));
  if (doTest("[Reverse] Groups 10", tests)) suite.addTest(new RexTest("[Reverse] Groups 10", RAW("\1\s+(\w+)"), "HellO hELLo", true, 2, results28, Rex::Reverse|Rex::NoCase));
  
  const char *results29[] = {"  mypicture_000tata_007tata_007", "tata_007"};
  const char *results30[] = {"tata_007tata_007", "tata_007"};
  const char *results31[] = {"1,2,3,4,5,6,7,8,9,10,11,P", "11,"};
  const char *results32[] = {"taro"};
  const char *results33[] = {"a"};
  const char *results34[] = {" taro"};
  const char *results35[] = {"\n taro"};
  const char *results36[] = {"True"};
  
  if (doTest("Alternative 0", tests)) suite.addTest(new RexTest("Alternative 0", RAW("true|ok|on|1"), "True", true, 1, results36, Rex::NoCase));
  if (doTest("Alternative 1", tests)) suite.addTest(new RexTest("Alternative 1", RAW("(?i:true|ok|on|1)"), "True", true, 1, results36));
  
  if (doTest("Complex 0", tests)) suite.addTest(new RexTest("Complex 0", RAW("^\s*([a-z]+_\d{3})*?\1$"), "  mypicture_000tata_007tata_007", true, 2, results29));
  if (doTest("Complex 1", tests)) suite.addTest(new RexTest("Complex 1", RAW("([a-z]+_\d{3})\1$"), "  mypicture_000tata_007tata_007", true, 2, results30));
  if (doTest("Complex 2", tests)) suite.addTest(new RexTest("Complex 2", RAW("^(.*?,){11}P"), "1,2,3,4,5,6,7,8,9,10,11,P", true, 2, results31));
  if (doTest("Complex 3", tests)) suite.addTest(new RexTest("Complex 3", RAW("^([^,\r\n]*,){11}P"), "1,2,3,4,5,6,7,8,9,10,11,P", true, 2, results31));
  if (doTest("Complex 4", tests)) suite.addTest(new RexTest("Complex 4", RAW("(?<=\d{3}[a-z0-9_-]o)taro"), "  taro, 001_otaro", true, 1, results32));
  if (doTest("Complex 5", tests)) suite.addTest(new RexTest("Complex 5", RAW("(?<=\bt)a"), " yotaro taro", true, 1, results33));
  if (doTest("Complex 6", tests)) suite.addTest(new RexTest("Complex 6", "(?<=^hello)\\staro", "hello taro", true, 1, results34));
  if (doTest("Complex 7", tests)) suite.addTest(new RexTest("Complex 7", "(?<=hello$)\r?\n\\staro", "hello\n taro", true, 1, results35, Rex::Multiline));
  if (doTest("Complex 8", tests)) suite.addTest(new RexTest("Complex 8", "(?<=frame\\.\\d+\\.exr$)\r?\n\\staro", "frame.1.exr\n taro", true, 1, results35, Rex::Multiline));
  
  const char *results37[] = {"hell"};
  
  if (doTest("Comment", tests)) suite.addTest(new RexTest("Comment", "(?#a comment)[a-z]+", "12hellO", true, 1, results37));
  
  const char *results38[] = {"$MAYA_VER", "$", "", "MAYA_VER"};
  const char *results39[] = {"%MAYA_VER%", "", "%", "MAYA_VER"};
  if (doTest("Condition 0", tests)) suite.addTest(new RexTest("Condition 0", RAW("(?:(?P<refunix>\$)|(?P<refwin>%))([a-zA-Z_][a-zA-Z0-9_]+)(?(refwin)(?P=refwin))"), "/usr/autodesk/maya$MAYA_VER/bin", true, 4, results38));
  if (doTest("Condition 1", tests)) suite.addTest(new RexTest("Condition 1", RAW("(?:(?P<refunix>\$)|(?P<refwin>%))([a-zA-Z_][a-zA-Z0-9_]+)(?(refwin)(?P=refwin))"), "/usr/autodesk/maya%MAYA_VER%/bin", true, 4, results39));
  if (doTest("Condition 2", tests)) suite.addTest(new RexTest("Condition 2", RAW("(?:(?P<refunix>\$)|(?P<refwin>%))([a-zA-Z_][a-zA-Z0-9_]+)(?(refwin)(?P=refwin))"), "/usr/autodesk/maya%MAYA_VER/bin", false, 0));
    
  suite.execute(true);
  
  std::cout << std::endl << "Named group substitution: " << std::endl;
  
  String s = "shape123";
  std::cout << s << " -> ";
  s.subst("(?P<name>[a-zA-Z][a-zA-Z_-]*)\\d+", "\\g<name>");
  std::cout << s << " (expected: \"shape\")" << std::endl;
  
  s = "anotherShape";
  std::cout << s << " -> ";
  s.subst("(?P<name>[a-zA-Z][a-zA-Z_-]*)\\d+", "\\g<name>");
  std::cout << s << " (expected: \"anotherShape\")" << std::endl;
  
  s = "aThirdShape3";
  std::cout << s << " -> ";
  s.subst("(?P<name>[a-zA-Z][a-zA-Z_-]*)\\d+", "\\g<dummy>");
  std::cout << s << " (expected: \"\\g<dummy>\")" << std::endl;
  
  return 0;
}


