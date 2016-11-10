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
   gcore::FlagDesc cmdflags[] = {ACCEPTS_NOFLAG_ARGUMENTS(2)};
   
   gcore::ArgParser args(cmdflags, 1);
   
   gcore::Status stat = args.parse(argc-1, argv+1);
   
   if (!stat)
   {
      std::cerr << stat << std::endl;
      std::cout << "writebom <path> utf-8|utf-16|utf-16be|utf-16le|utf-32|utf-32be|utf-32le|none" << std::endl;
      return 1;
   }
   
   unsigned char BOM[4];
   size_t bs = 0;
   bool rem = false;
   gcore::String bomstr;
   
   args.getArgument(1, bomstr);

   if (bomstr == "utf-8")
   {
      // utf-8 encoding of 0xFEFF
      BOM[0] = 0xEF;
      BOM[1] = 0xBB;
      BOM[2] = 0xBF;
      bs = 3;
   }
   else if (bomstr == "utf-16" || bomstr == "utf-16be")
   {
      BOM[0] = 0xFE;
      BOM[1] = 0xFF;
      bs = 2;
   }
   else if (bomstr == "utf-16le")
   {
      BOM[0] = 0xFF;
      BOM[1] = 0xFE;
      bs = 2;
   }
   else if (bomstr == "utf-32" || bomstr == "utf-32be")
   {
      BOM[0] = 0x00;
      BOM[1] = 0x00;
      BOM[2] = 0xFE;
      BOM[3] = 0xFF;
      bs = 4;
   }
   else if (bomstr == "utf-32le")
   {
      BOM[0] = 0xFF;
      BOM[1] = 0xFE;
      BOM[2] = 0x00;
      BOM[3] = 0x00;
      bs = 4;
   }
   else if (bomstr == "none")
   {
      rem = true;
   }
   else
   {
      std::cout << "Invalid BOM \"" << argv[2] << "\"" << std::endl;
      return 1;
   }
   
   std::cout << "Target BOM: ";
   for (size_t i=0; i<bs; ++i)
   {
      std::cout << std::hex << (unsigned short)BOM[i] << std::dec << " ";
   }
   std::cout << std::endl;
   
   gcore::String filename;
   
   args.getArgument(0, filename);
   
   gcore::Path path(filename);
   
   FILE *f = path.open("rb");
   
   if (!f)
   {
      std::cout << "Invalid file \"" << argv[1] << "\"" << std::endl;
      return 1;
   }
   
   unsigned char curBOM[4];
   size_t cbs = 0;
   size_t n = fread(curBOM, 1, 4, f);
   
   std::cout << "File first " << n << " byte(s): ";
   for (size_t i=0; i<n; ++i)
   {
      std::cout << std::hex << (unsigned short)curBOM[i] << std::dec << " ";
   }
   std::cout << std::endl;
   
   if (n >= 2)
   {
      if (curBOM[0] == 0xFF && curBOM[1] == 0xFE)
      {
         if (n == 4 && curBOM[2] == 0 && curBOM[3] == 0)
         {
            std::cout << "Current BOM: utf-32le" << std::endl;
            cbs = 4;
         }
         else
         {
            std::cout << "Current BOM: utf-16le" << std::endl;
            cbs = 2;
         }
      }
      else if (curBOM[0] == 0xFE && curBOM[1] == 0xFF)
      {
         std::cout << "Current BOM: utf-16(be)" << std::endl;
         cbs = 2;
      }
      else if (n >= 3)
      {
         if (curBOM[0] == 0xEF && curBOM[1] == 0xBB && curBOM[2] == 0xBF)
         {
            std::cout << "Current BOM: utf-8" << std::endl;
            cbs = 3;
         }
         else if (n == 4 && curBOM[0] == 0 && curBOM[1] == 0 && curBOM[2] == 0xFE && curBOM[3] == 0xFF)
         {
            std::cout << "Current BOM: utf-32(be)" << std::endl;
            cbs = 4;
         }
      }
   }
   
   // Check if file has to be modified
   if (rem)
   {
      if (cbs == 0)
      {
         std::cout << "No BOM to remove" << std::endl;
         fclose(f);
         return 0;
      }
   }
   else
   {
      if (cbs == bs)
      {
         bool changed = false;
         
         for (size_t i=0; i<bs; ++i)
         {
            if (curBOM[i] != BOM[i])
            {
               changed = true;
               break;
            }
         }
         
         if (!changed)
         {
            std::cout << "BOM already set to " << argv[2] << std::endl;
            fclose(f);
            return 0;
         }
      }
   }
   
   // Read current file content
   fseek(f, 0, SEEK_END);
   size_t fs = size_t(ftell(f));
   unsigned char *content = (unsigned char*) malloc(fs);
   fseek(f, 0, SEEK_SET);
   n = fread(content, 1, fs, f);
   fclose(f);
   
   if (n != fs)
   {
      std::cout << "Failed to read file content" << std::endl;
      return 1;
   }
   else
   {
      f = path.open("wb");
      if (!rem)
      {
         fwrite(BOM, 1, bs, f);
      }
      fwrite(content + cbs, 1, fs - cbs, f);
      fclose(f);
   }
   
   free(content);

   return 0;
}
