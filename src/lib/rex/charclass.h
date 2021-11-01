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

#ifndef __gcore_rex_charclass__
#define __gcore_rex_charclass__

#define ESCAPE_CHAR  0x01
#define DIGIT_CHAR   0x02
#define WORD_CHAR    0x04
#define HEXA_CHAR    0x08
#define SPACE_CHAR   0x10
#define SPECIAL_CHAR 0x20
#define UPPER_CHAR   0x40
#define LOWER_CHAR   0x80
#define LETTER_CHAR  LOWER_CHAR|UPPER_CHAR

#define EOS '\0'
#define ESC '\\'
#define HEX '\x'
#define CTL '\c'
#define BCK '\b'

namespace gcore
{
   extern unsigned char gCharClass[256];
}

#define CHAR_IS(c, flags) ((gCharClass[(unsigned char)c] & (flags)) != 0)


#endif


