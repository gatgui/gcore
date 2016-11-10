/*

Copyright (C) 2016~  Gaetan Guidet

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

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <gcore/argparser.h>
#include <gcore/path.h>

int main(int argc, char **argv)
{
   gcore::FlagDesc cmdflags[] = {ACCEPTS_NOFLAG_ARGUMENTS(1)};
   
   gcore::ArgParser args(cmdflags, 1);
   
   gcore::Status stat = args.parse(argc-1, argv+1);
   
   if (!stat)
   {
      std::cerr << stat << std::endl;
      return 1;
   }
   
   gcore::String filename;
   
   args.getArgument(0, filename);
   
   gcore::Path path(filename);
   
   FILE *f = path.open("rb");
   
   if (!f)
   {
      std::cout << "Invalid file \"" << argv[1] << "\"" << std::endl;
      return 1;
   }
   
   unsigned char BOM[4];
   bool found = false;   
   size_t n = fread(BOM, 1, 4, f);
   
   if (n >= 2)
   {
      if (BOM[0] == 0xFF && BOM[1] == 0xFE)
      {
         if (n == 4 && BOM[2] == 0 && BOM[3] == 0)
         {
            std::cout << "utf-32le" << std::endl;
         }
         else
         {
            std::cout << "utf-16le" << std::endl;
         }
         found = true;
      }
      else if (BOM[0] == 0xFE && BOM[1] == 0xFF)
      {
         std::cout << "utf-16/utf-16be" << std::endl;
         found = true;
      }
      else if (n >= 3)
      {
         if (BOM[0] == 0xEF && BOM[1] == 0xBB && BOM[2] == 0xBF)
         {
            std::cout << "utf-8" << std::endl;
            found = true;
         }
         else if (n == 4 && BOM[0] == 0 && BOM[1] == 0 && BOM[2] == 0xFE && BOM[3] == 0xFF)
         {
            std::cout << "utf-32/utf-32be" << std::endl;
            found = true;
         }
      }
   }
   
   if (!found)
   {
      std::cout << "none" << std::endl;
   }
   
   fclose(f);

   return 0;
}
