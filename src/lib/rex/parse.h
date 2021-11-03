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

#ifndef __gcore_rex_parse_h_
#define __gcore_rex_parse_h_

namespace gcore {

class Instruction;
class Repeat;

struct ParseInfo
{
  int numGroups;
  // nothing else?
  // if we had named group -> group name to group index mapping
};

extern bool ReadDecimal(const char **ppc, unsigned long &v);
extern bool ReadHexadecimal(const char **ppc, unsigned char &v);
extern bool ReadOctal(const char **ppc, unsigned char &v);

extern Instruction* ParseExpression(const char **ppc, ParseInfo &info);
extern Instruction* ParseBranch(const char **ppc, ParseInfo &info);
extern Instruction* ParsePiece(const char **ppc, ParseInfo &info);
extern Instruction* ParseAtom(const char **ppc, ParseInfo &info);
extern Instruction* ParseCharacters(const char **ppc, ParseInfo &info);
extern Instruction* ParseRange(const char **ppc, bool inv, ParseInfo &info);
extern Repeat* ParseRepeat(const char **ppc, bool &err, ParseInfo &info);
extern Instruction* ParseZerowidth(const char **ppc, ParseInfo &info);

}

#endif
