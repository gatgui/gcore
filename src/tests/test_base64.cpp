/*
MIT License

Copyright (c) 2013 Gaetan Guidet

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

#include <gcore/base64.h>

int main(int argc, char **argv)
{
   if (argc == 1)
   {
      float v0[4] = {-100.2f, 25.2349f, -0.291625f, 0.0001f};
      float v1[4] = {0, 0, 0, 0};
      
      std::cout << v0[0] << ", " << v0[1] << ", " << v0[2] << ", " << v0[3] << std::endl;
      
      gcore::String es = gcore::base64::Encode(v0, 4*sizeof(float));
      std::cout << es << std::endl;
      
      gcore::base64::Decode(es, v1, 4*sizeof(float));
      std::cout << v1[0] << ", " << v1[1] << ", " << v1[2] << ", " << v1[3] << std::endl;
      
      return 0;
   }
   
   if (argc != 3)
   {
      std::cout << "Usage: test_base64 ((--decode|--encode) <string>)?" << std::endl;
      return -1;
   }
   
   gcore::String str = argv[2];
   
   if (strcmp(argv[1], "--decode") == 0)
   {
      gcore::String dec = gcore::base64::Decode(str);
      std::cout << "\"" << str << "\" -> \"" << dec << "\"" << std::endl;
   }
   else if (strcmp(argv[1], "--encode") == 0)
   {
      gcore::String enc = gcore::base64::Encode(str);
      std::cout << "\"" << str << "\" -> \"" << enc << "\"" << std::endl;
   }
   else
   {
      std::cout << "Invalid options: " << argv[1] << std::endl;
   }
   
   return 0;
}
