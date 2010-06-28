/*

Copyright (C) 2010  Gaetan Guidet

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

#include <gcore/string.h>
#include <gcore/file.h>

int main(int, char**) {
  
  std::string s0 = "Z:\\ve\\home/GaetanG/dev\\deploy";
  std::string s1 = gcore::String::Replace('\\', gcore::JoinPath(s0, "\tPoo  "), '/');
  std::string s2 = gcore::String::Replace("GaetanG", s1, "PetitN");
  std::vector<std::string> splits;
  gcore::String::Split(s2, '/', splits);
  for (size_t i=0; i<splits.size(); ++i) {
    splits[i] = gcore::String::Strip(splits[i]);
  }
  
  std::cout << "s0 = \"" << s0 << "\"" << std::endl;
  std::cout << "s1 = \"" << s1 << "\"" << std::endl;
  std::cout << "s2 = \"" << s2 << "\"" << std::endl;
  std::cout << "ToUpper s2:" << std::endl;
  std::cout << "   = \"" << gcore::String::ToUpper(s2) << "\"" << std::endl;
  std::cout << "ToLower s2:" << std::endl;
  std::cout << "   = \"" << gcore::String::ToLower(s2) << "\"" << std::endl;
  std::cout << "Split Strip s1:" << std::endl;
  for (size_t i=0; i<splits.size(); ++i) {
    std::cout << "  \"" << splits[i] << "\"" << std::endl;
  }
  
  return 0;
}
