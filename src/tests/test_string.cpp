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
#include <gcore/path.h>

void QuoteString(gcore::String &s) {
  s = "\"" + s + "\"";
}

int main(int, char**) {
  
  gcore::String s0 = "Z:\\ve\\home/GaetanG/dev\\deploy";
  
  gcore::String s1 = s0 + "/\tPoo  ";
  s1.replace('\\', '/');
  
  gcore::String s2 = s1;
  s2.replace("GaetanG", "PetitN");
  
  gcore::StringList splits;
  s2.split('/', splits);
  for (size_t i=0; i<splits.size(); ++i) {
    splits[i].strip();
  }
  
  std::cout << "s0 = \"" << s0 << "\"" << std::endl;
  std::cout << "s1 = \"" << s1 << "\"" << std::endl;
  std::cout << "s2 = \"" << s2 << "\"" << std::endl;
  std::cout << "ToUpper s2:" << std::endl;
  std::cout << "   = \"" << s2.toupper() << "\"" << std::endl;
  std::cout << "ToLower s2:" << std::endl;
  std::cout << "   = \"" << s2.tolower() << "\"" << std::endl;
  std::cout << "Split Strip s1:" << std::endl;
  
  gcore::StringList::MapFunc quote;
  gcore::Bind(QuoteString, quote);
  std::cout << splits.map(quote) << std::endl;
  
  return 0;
}
