/*
MIT License

Copyright (c) 2016 Gaetan Guidet

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

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <gcore/argparser.h>
#include <gcore/path.h>
#include <gcore/unicode.h>

int main(int argc, char **argv)
{
   gcore::FlagDesc cmdflags[] = {ACCEPTS_NOFLAG_ARGUMENTS(2)};
   
   gcore::ArgParser args(cmdflags, 1);
   
   gcore::Status stat = args.parse(argc-1, argv+1);
   
   if (!stat)
   {
      std::cerr << stat << std::endl;
      std::cout << "writebom <path> utf-8|utf-16|utf-16le|utf-32|utf-32le|none" << std::endl;
      return 1;
   }
   
   bool rem = false;
   gcore::String bomstr;
   
   args.getArgument(1, bomstr);
   
   gcore::Encoding bom = gcore::StringToEncoding(bomstr.c_str());
   
   switch (bom)
   {
   case gcore::UTF_8:
   case gcore::UTF_16BE:
   case gcore::UTF_16LE:
   case gcore::UTF_32BE:
   case gcore::UTF_32LE:
      break;
   default:
      if (bomstr == "none")
      {
         rem = true;
      }
      else
      {
         std::cout << "Invalid BOM \"" << argv[2] << "\"" << std::endl;
         return 1;
      }
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
   
   gcore::Encoding cbom = gcore::ReadBOM(f);
   size_t cbs = gcore::BOMSize(cbom);
   const char *cbomstr = gcore::EncodingToString(cbom);
   
   std::cout << "Current BOM: " << (cbomstr ? cbomstr : "unknown") << std::endl;
   
   // Check if file has to be modified
   if (rem)
   {
      if (cbom == gcore::INVALID_ENCODING)
      {
         std::cout << "No BOM to remove" << std::endl;
         fclose(f);
         return 0;
      }
   }
   else
   {
      if (cbom == bom)
      {
         std::cout << "BOM already set to " << argv[2] << std::endl;
         fclose(f);
         return 0;
      }
   }
   
   // Read current file content
   fseek(f, 0, SEEK_END);
   size_t fs = size_t(ftell(f));
   unsigned char *content = (unsigned char*) malloc(fs);
   fseek(f, 0, SEEK_SET);
   size_t n = fread(content, 1, fs, f);
   fclose(f);
   
   if (n != fs)
   {
      std::cout << "Failed to read file content" << std::endl;
      return 1;
   }
   else
   {
      f = path.open("wb");
      if (cbs > 0)
      {
         std::cout << "Remove BOM" << std::endl;
      }
      if (!rem)
      {
         std::cout << "Write BOM: " << bomstr << std::endl;
         gcore::WriteBOM(f, bom);
      }
      fwrite(content + cbs, 1, fs - cbs, f);
      fclose(f);
   }
   
   free(content);

   return 0;
}
