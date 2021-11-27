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
   
   gcore::Encoding e = gcore::ReadBOM(filename.c_str());
   
   const char *es = gcore::EncodingToString(e);
   
   std::cout << (es ? es : "unknown") << std::endl;
   
   return 0;
}