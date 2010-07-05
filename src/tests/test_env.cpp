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

#include <gcore/env.h>
#include <gcore/path.h>

bool PathCallback(const gcore::Path &path) {
  std::cout << "  \"" << path.fullname() << "\"" << std::endl;
  return true;
}

int main(int, char**) {
  gcore::StringDict all;
  gcore::StringDict::iterator it;
  
  gcore::Env env;
  
  env.asDict(all);
  it = all.begin();
  while (it != all.end()) {
    std::cout << it->first << "=" << it->second << std::endl;
    ++it;
  }
  
  std::cout << "Check env var BSROOT..." << std::endl;
  
  std::string bsroot = "Z:/ve/home/GaetanG/dev/deploy/projects";
  
  if (env.isSet("BSROOT")) {
    std::cout << "BSROOT already set: \"" << env.get("BSROOT") << "\"" << std::endl;
    env.set("BSROOT", bsroot, true);
  } else {
    env.set("BSROOT", bsroot, false);
  }
  std::cout << "BSROOT = \"" << env.get("BSROOT") << "\"" << std::endl;
  
  std::cout << "PATH content..." << std::endl;
  gcore::Env::EachInPathFunc cb;
  gcore::Bind(PathCallback, cb);
  gcore::Env::EachInPath("PATH", cb);
  
  std::cout << "PATH content... (using PathList)" << std::endl;
  gcore::PathList pl;
  gcore::Env::ListPaths("PATH", pl);
  std::cout << pl << std::endl;
  
  return 0;
}

