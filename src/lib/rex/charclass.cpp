/*

Copyright (C) 2009, 2010  Gaetan Guidet

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

#include <stdlib.h>
#include <string.h>
#include "charclass.h"

namespace gcore {

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

}
