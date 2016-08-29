/*

Copyright (C) 2009, 2010  Gaetan Guidet

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

#include <gcore/plist.h>
#include <gcore/json.h>

int main(int argc, char **argv) {
  if (argc != 2) {
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
        !(stat = spl.setString("aaa.bbb1.3[2].ccc.d2.4", "6.8"))) {
      std::cerr << stat.message() << std::endl;
      return -1;
    }
    spl.write(filename);
  }
  
  gcore::PropertyList pl;
  {
    std::cout << "Read plist from file..." << std::endl;
    
    if (!pl.read(filename)) {
      std::cerr << "Could not read file: \"" << filename << "\"" << std::endl;
      return -1;
    }
    
    gcore::json::Value json;
    if (pl.toJSON(json)) {
      std::cout << "Write JSON file..." << std::endl;
      json.write("out.json");
    }
    
    // This will be ambiguous
    //pl.getInteger("font.size", 0);
    // Use:
    pl.getInteger("font.size", long(0));
    // Or:
    //long defaultValue = 0;
    //pl.getInteger("font.size", defaultValue);
    
    std::cout << "Font name: " << pl.getString("font.name", &stat) << " (" << stat << ")" << std::endl;
    std::cout << "Font size: " << pl.getInteger("font.size", &stat) << " (" << stat << ")" << std::endl;
    std::cout << "Num schemes: " << pl.getSize("schemes.all", &stat) << " (" << stat << ")" << std::endl;
    std::cout << "schemes.all[2].name = " << pl.getString("schemes.all[2].name", &stat) << " (" << stat << ")" << std::endl;
    std::cout << "aaa.bbb1.2.ccc.d2.4 = " << pl.getString("aaa.bbb1.2.ccc.d2.4", &stat) << " (" << stat << ")" << std::endl;
    std::cout << "aaa.bbb1.3[0].ccc.d2.4 = " << pl.getString("aaa.bbb1.3[0].ccc.d2.4", &stat) << " (" << stat << ")" << std::endl;

    pl.remove("aaa.bbb1.2.ccc.d2.4");
    pl.remove("schemes.all[1]");
    
    std::cout << "Num schemes: " << pl.getSize("schemes.all", &stat) << " (" << stat << ")" << std::endl;
    std::cout << "schemes.all[0].name = " << pl.getString("schemes.all[0].name", &stat) << " (" << stat << ")" << std::endl;
    std::cout << "schemes.all[1].name = " << pl.getString("schemes.all[1].name", &stat) << " (" << stat << ")" << std::endl;
    std::cout << "schemes.all[2].name = " << pl.getString("schemes.all[2].name", &stat) << " (" << stat << ")" << std::endl; 
    
  }
  pl.write("test.xml");
}

