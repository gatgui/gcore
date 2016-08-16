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

#include <gcore/all.h>


int main(int, char**) {
  
  gcore::String exp = "fred(?!eric)";
  gcore::Rex re0(exp);
  
  gcore::StringList strs;
  strs.push_back("frederic");
  strs.push_back("fredo");
  
  for (size_t i=0; i<strs.size(); ++i) {
    std::cout << "\"" << strs[i] << "\" =~ /" << exp << "/" << std::endl;
    if (re0.match(strs[i])) {
      std::cout << "  Matched" << std::endl;
    } else {
      std::cout << "  Not matched" << std::endl;
    }
  }
  
  gcore::Rex re1(RAW("(\d\d):(\d\d)"));
  
  //fprintf(stderr, "Error? %s\n", re1.getError(err));
  
  gcore::RexMatch md;
  
  gcore::String str = "   Time -> 10:23  ";
  
  if (re1.search(str, md, 0, 3, str.length()-3))
  {
    fprintf(stderr, "\"%s",             md.pre().c_str() );
    fprintf(stderr, "<<%s>>",           md.group(0).c_str()  );
    fprintf(stderr, "%s\"\n",           md.post().c_str());
    fprintf(stderr, "Found1: \'%s\'\n", md.group(1).c_str()  );
    fprintf(stderr, "Found2: \'%s\'\n", md.group(2).c_str()  );
    
    //fprintf(stderr, "Substitute: %s\n", re1.substitute(md, "Current time is \\1:\\2, removed \"\\`\" and \"\\'\", whole match \"\\&\"").c_str());
    fprintf(stderr, "Substitute: %s\n", re1.substitute(md, RAW("Current time is \1:\2, removed '\`' and '\'', whole match '\&'")).c_str());
  }
  else
  {
    fprintf(stderr, "Not found\n");
  }
  
  return 0;
}
