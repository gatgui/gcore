/*
MIT License

Copyright (c) 2009 Gaetan Guidet

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

#include <gcore/path.h>
#include <gcore/argparser.h>

int main(int argc, char **argv)
{
   gcore::FlagDesc flags[] = {ACCEPTS_NOFLAG_ARGUMENTS(1)};
   gcore::ArgParser args(flags, 1);
   
   gcore::Status stat = args.parse(argc-1, argv+1);
   if (!stat)
   {
      std::cerr << stat << std::endl;
      return 1;
   }
   
   gcore::String str;
   args.getArgument(0, str);
   
   std::cout << "Read file '" << str << "'" << std::endl;
   
   gcore::Path path(str);
   
   if (!path.isFile())
   {
      std::cerr << "Not a file '" << str << "'" << std::endl;
      return 1;
   }
   
   std::cout << "Page size: " << gcore::MMap::PageSize() << std::endl;
   
   gcore::MMap mmf;
   
   std::cout << "Map file to memory" << std::endl;
   
   stat = mmf.open(path, gcore::MMap::READ_ONLY, 20);
   if (!stat)
   {
      std::cerr << stat << std::endl;
      return 1;
   }
   
   std::cout << "Open successfully: size = " << mmf.size() << " [file size = " << path.fileSize() << "]" << std::endl;
   
   for (size_t i=0; i<mmf.size(); ++i)
   {
      std::cout << mmf[i];
   }
   
   std::cout << std::endl;
   
   return 0;
}
