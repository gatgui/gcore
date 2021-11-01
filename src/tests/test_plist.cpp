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

#include <gcore/plist.h>
#include <gcore/json.h>

int main(int argc, char **argv)
{
   if (argc != 2)
   {
      std::cout << "Usage: test_plist <filename>" << std::endl;
      return -1;
   }
   
   gcore::Status stat;
   gcore::String filename(argv[1]);
   
   { 
      std::cout << "Create and write plist to file..." << std::endl;
      
      gcore::PropertyList spl;
      spl.create();
      if (!(stat = spl.setString("font.name", "Helvetica")) ||
          !(stat = spl.setInteger("font.size", 16)) ||
          !(stat = spl.setString("schemes.all[0].name", "Hello")) ||
          !(stat = spl.setString("schemes.all[1].name", "World")) ||
          !(stat = spl.setString("schemes.all[2].name", "Goodbye")) ||
          !(stat = spl.setString("aaa.bbb1.2.ccc.d2.4", "3.4")) ||
          !(stat = spl.setString("aaa.bbb1.3[2].ccc.d2.4", "6.8")))
      {
         std::cerr << stat.message() << std::endl;
         return -1;
      }
      spl.write(filename);
   }
   
   gcore::PropertyList pl;
   {
      std::cout << "Read plist from file..." << std::endl;
      
      if (!pl.read(filename))
      {
         std::cerr << "Could not read file: \"" << filename << "\"" << std::endl;
         return -1;
      }
      
      gcore::json::Value json;
      if (pl.toJSON(json))
      {
         std::cout << "Write JSON file..." << std::endl;
         json.write("out.json");
      }
      
      //pl.getInteger("font.size", 0);
      
      std::cout << "Font name: " << pl.asString("font.name", &stat) << " (" << stat << ")" << std::endl;
      std::cout << "Font size: " << pl.asInteger("font.size", &stat) << " (" << stat << ")" << std::endl;
      std::cout << "Num schemes: " << pl.size("schemes.all", &stat) << " (" << stat << ")" << std::endl;
      std::cout << "schemes.all[2].name = " << pl.asString("schemes.all[2].name", &stat) << " (" << stat << ")" << std::endl;
      std::cout << "aaa.bbb1.2.ccc.d2.4 = " << pl.asString("aaa.bbb1.2.ccc.d2.4", &stat) << " (" << stat << ")" << std::endl;
      std::cout << "aaa.bbb1.3[0].ccc.d2.4 = " << pl.asString("aaa.bbb1.3[0].ccc.d2.4", &stat) << " (" << stat << ")" << std::endl;

      pl.remove("aaa.bbb1.2.ccc.d2.4");
      pl.remove("schemes.all[1]");
      
      std::cout << "Num schemes: " << pl.size("schemes.all", &stat) << " (" << stat << ")" << std::endl;
      std::cout << "schemes.all[0].name = " << pl.asString("schemes.all[0].name", &stat) << " (" << stat << ")" << std::endl;
      std::cout << "schemes.all[1].name = " << pl.asString("schemes.all[1].name", &stat) << " (" << stat << ")" << std::endl;
      std::cout << "schemes.all[2].name = " << pl.asString("schemes.all[2].name", &stat) << " (" << stat << ")" << std::endl; 
      
   }
   pl.write("test.xml");
}

