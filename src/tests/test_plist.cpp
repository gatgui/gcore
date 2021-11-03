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

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "Usage: test_plist <filename>" << std::endl;
    return -1;
  }
  
  gcore::String filename(argv[1]);
  
  std::cout << "Create and write plist to file..." << std::endl;
  try {
    gcore::PropertyList spl;
    spl.create();
    spl.setString("font.name", "Helvetica");
    spl.setInteger("font.size", 16);
    spl.setString("schemes.all[0].name", "Hello");
    spl.setString("schemes.all[1].name", "World");
    spl.setString("schemes.all[2].name", "Goodbye");
    spl.setString("aaa.bbb1.2.ccc.d2.4", "3.4");
    spl.setString("aaa.bbb1.3[2].ccc.d2.4", "6.8");
    spl.write(filename);
  } catch (gcore::plist::Exception &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
  
  std::cout << "Read plist from file..." << std::endl;
  
  gcore::PropertyList pl;
  
  try {
    if (!pl.read(filename)) {
      std::cerr << "Could not read file: \"" << filename << "\"" << std::endl;
      return -1;
    }
    
    gcore::json::Value json;
    if (pl.toJSON(json)) {
      std::cout << "Write JSON file..." << std::endl;
      json.write("out.json");
    }
    
    std::cout << "Font name: " << pl.getString("font.name") << std::endl;
    std::cout << "Font size: " << pl.getInteger("font.size") << std::endl;
    std::cout << "Num schemes: " << pl.getSize("schemes.all") << std::endl;
    std::cout << "schemes.all[2].name = " << pl.getString("schemes.all[2].name") << std::endl;
    std::cout << "aaa.bbb1.2.ccc.d2.4 = " << pl.getString("aaa.bbb1.2.ccc.d2.4") << std::endl;
    // there were only one element in bbb1.3 array, after serialization, index are adjusted
    std::cout << "aaa.bbb1.3[0].ccc.d2.4 = " << pl.getString("aaa.bbb1.3[0].ccc.d2.4") << std::endl;

    pl.remove("aaa.bbb1.2.ccc.d2.4");
    pl.remove("schemes.all[1]");
    std::cout << "Num schemes: " << pl.getSize("schemes.all") << std::endl;
    //pl.remove("schemes.all");
    //std::cout << "aaa.bbb1.2.ccc.d2.4 = " << pl.getString("aaa.bbb1.2.ccc.d2.4") << std::endl;
    std::cout << "schemes.all[0].name = " << pl.getString("schemes.all[0].name") << std::endl;
    std::cout << "schemes.all[1].name = " << pl.getString("schemes.all[1].name") << std::endl;
    std::cout << "schemes.all[2].name = " << pl.getString("schemes.all[2].name") << std::endl;
    /*
    std::cout << pl.getReal("projection[0][3]") << std::endl;
    std::cout << pl.getReal("projection[1][3]") << std::endl;
    std::cout << pl.getReal("projection[2][3]") << std::endl;
    std::cout << pl.getReal("projection[3][3]") << std::endl;
    std::cout << pl.getString("projection[4][0].type") << std::endl;
    std::cout << pl.getSize("projection") << std::endl;
    std::cout << pl.getSize("projection[0]") << std::endl;
    std::cout << pl.getSize("projection[4]") << std::endl;
    
    pl.setString("fonts[4][2][1].desc[0].name", "Monaco");
    std::cout << pl.getString("fonts[4][2][1].desc[0].name") << std::endl;
    */
  
  } catch (gcore::plist::Exception &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }

  pl.write("test.xml");
}

