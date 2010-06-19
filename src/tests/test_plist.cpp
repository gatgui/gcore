/*

Copyright (C) 2009  Gaetan Guidet

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

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "Usage: test_plist <filename>" << std::endl;
    return -1;
  }
  
  std::string filename(argv[1]);
  
  gcore::PropertyList pl;
  
  if (!pl.read(filename)) {
    std::cerr << "Could not read file: \"" << filename << "\"" << std::endl;
    return -1;
  }
  
  try {
    std::cout << "Font name: " << pl.getString("font.name") << std::endl;
    std::cout << "Font size: " << pl.getInteger("font.size") << std::endl;
    std::cout << "Num schemes: " << pl.getArraySize("schemes.all") << std::endl;
    std::cout << "schemes.all[2].name = " << pl.getString("schemes.all[2].name") << std::endl;
    
    /*
    std::cout << pl.getReal("projection[0][3]") << std::endl;
    std::cout << pl.getReal("projection[1][3]") << std::endl;
    std::cout << pl.getReal("projection[2][3]") << std::endl;
    std::cout << pl.getReal("projection[3][3]") << std::endl;
    std::cout << pl.getString("projection[4][0].type") << std::endl;
    std::cout << pl.getArraySize("projection") << std::endl;
    std::cout << pl.getArraySize("projection[0]") << std::endl;
    std::cout << pl.getArraySize("projection[4]") << std::endl;
    
    pl.setString("fonts[4][2][1].desc[0].name", "Monaco");
    std::cout << pl.getString("fonts[4][2][1].desc[0].name") << std::endl;
    */
  
  } catch (gcore::plist::Exception &e) {
    std::cout << e.what() << std::endl;
  }

  pl.write("test.xml");
}

