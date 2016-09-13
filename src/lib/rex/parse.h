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

#ifndef __gcore_rex_parse_h_
#define __gcore_rex_parse_h_

namespace gcore
{
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
