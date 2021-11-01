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

#include <stdlib.h>
#include <string.h>
#include "charclass.h"

namespace gcore
{

unsigned char gCharClass[256];

class CharClassInitializer
{
public:
   
   CharClassInitializer()
   {
      unsigned char c;
      
      memset(gCharClass, 0, 256);
      
      for (c='a'; c<='z'; ++c) {
         gCharClass[c] = LOWER_CHAR|WORD_CHAR;
      }
      for (c='A'; c<='Z'; ++c) {
         gCharClass[c] = UPPER_CHAR|WORD_CHAR;
      }
      gCharClass[(unsigned char)'_'] = WORD_CHAR;
      
      for (c='0'; c<='9'; ++c) {
         gCharClass[c] = DIGIT_CHAR|HEXA_CHAR;
      }
      
      for (c='a'; c<='f'; ++c) {
         gCharClass[c] |= HEXA_CHAR;
      }
      for (c='A'; c<='F'; ++c) {
         gCharClass[c] |= HEXA_CHAR;
      }
      
      gCharClass[(unsigned char)'\t'] = SPACE_CHAR|ESCAPE_CHAR;
      gCharClass[(unsigned char)'\n'] = SPACE_CHAR|ESCAPE_CHAR;
      gCharClass[(unsigned char)'\r'] = SPACE_CHAR|ESCAPE_CHAR;
      gCharClass[(unsigned char)'\f'] = SPACE_CHAR|ESCAPE_CHAR;
      gCharClass[(unsigned char)'\v'] = SPACE_CHAR|ESCAPE_CHAR;
      gCharClass[(unsigned char)' ']  = SPACE_CHAR;
      
      //gCharClass[(unsigned char)'\t'] = ESCAPE_CHAR;
      //gCharClass[(unsigned char)'\n'] = ESCAPE_CHAR;
      //gCharClass[(unsigned char)'\r'] = ESCAPE_CHAR;
      //gCharClass[(unsigned char)'\f'] = ESCAPE_CHAR;
      //gCharClass[(unsigned char)'\v'] = ESCAPE_CHAR;
      gCharClass[(unsigned char)'\b'] = ESCAPE_CHAR;
      gCharClass[(unsigned char)'\a'] = ESCAPE_CHAR;
      //gCharClass[(unsigned char)'\e'] = ESCAPE_CHAR;
      
      gCharClass[(unsigned char)'('] = SPECIAL_CHAR;
      gCharClass[(unsigned char)')'] = SPECIAL_CHAR;
      gCharClass[(unsigned char)'['] = SPECIAL_CHAR;
      gCharClass[(unsigned char)']'] = SPECIAL_CHAR;
      gCharClass[(unsigned char)'{'] = SPECIAL_CHAR;
      gCharClass[(unsigned char)'}'] = SPECIAL_CHAR;
      gCharClass[(unsigned char)'*'] = SPECIAL_CHAR;
      gCharClass[(unsigned char)'+'] = SPECIAL_CHAR;
      gCharClass[(unsigned char)'?'] = SPECIAL_CHAR;
      gCharClass[(unsigned char)'.'] = SPECIAL_CHAR;
      gCharClass[(unsigned char)'^'] = SPECIAL_CHAR;
      gCharClass[(unsigned char)'$'] = SPECIAL_CHAR;
      gCharClass[(unsigned char)'|'] = SPECIAL_CHAR;
      //gCharClass[(unsigned char)'\\'] = SPECIAL_CHAR;
   }
   
   ~CharClassInitializer()
   {
   }
};

static CharClassInitializer gsCharClassInit;

} // gcore
