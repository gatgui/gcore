/*

Copyright (C) 2013  Gaetan Guidet

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

#include <gcore/base64.h>

int main(int argc, char **argv) {
  
  if (argc == 1) {
    float v0[4] = {-100.2f, 25.2349f, -0.291625f, 0.0001f};
    float v1[4] = {0, 0, 0, 0};
    
    std::cout << v0[0] << ", " << v0[1] << ", " << v0[2] << ", " << v0[3] << std::endl;
    
    std::string es = gcore::base64::Encode(v0, 4*sizeof(float));
    std::cout << es << std::endl;
    
    gcore::base64::Decode(es, v1, 4*sizeof(float));
    std::cout << v1[0] << ", " << v1[1] << ", " << v1[2] << ", " << v1[3] << std::endl;
    
    return 0;
  }
  
  if (argc != 3) {
    std::cout << "Usage: test_base64 ((--decode|--encode) <string>)?" << std::endl;
    return -1;
  }
  
  std::string str = argv[2];
  
  if (strcmp(argv[1], "--decode") == 0) {
    std::string dec = gcore::base64::Decode(str);
    std::cout << "\"" << str << "\" -> \"" << dec << "\"" << std::endl;
  
  } else if (strcmp(argv[1], "--encode") == 0) {
    std::string enc = gcore::base64::Encode(str);
    std::cout << "\"" << str << "\" -> \"" << enc << "\"" << std::endl;
  
  } else {
    std::cout << "Invalid options: " << argv[1] << std::endl;
  }
  
  return 0;
}
