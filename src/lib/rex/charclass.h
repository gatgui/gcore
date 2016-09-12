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


