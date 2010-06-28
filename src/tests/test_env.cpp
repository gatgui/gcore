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

bool PathCallback(const std::string &val) {
  std::cout << "  \"" << val << "\"" << std::endl;
  return true;
}

int main(int, char**) {
  gcore::EnvDict all;
  gcore::EnvDict::iterator it;
  
  gcore::GetEnv(all);
  it = all.begin();
  while (it != all.end()) {
    std::cout << it->first << "=" << it->second << std::endl;
    ++it;
  }
  
  std::cout << "Check env var BSROOT..." << std::endl;
  
  std::string bsroot = "Z:/ve/home/GaetanG/dev/deploy/projects";
  
  if (gcore::HasEnv("BSROOT")) {
    std::cout << "BSROOT already set: \"" << gcore::GetEnv("BSROOT") << "\"" << std::endl;
    gcore::SetEnv("BSROOT", bsroot, true);
  } else {
    gcore::SetEnv("BSROOT", bsroot, false);
  }
  std::cout << "BSROOT = \"" << gcore::GetEnv("BSROOT") << "\"" << std::endl;
  
  std::cout << "PATH content..." << std::endl;
  gcore::EnumEnvCallback cb;
  gcore::MakeCallback(PathCallback, cb);
  gcore::ForEachInEnv("PATH", cb);
  
  std::cout << "... or using EnvList..." << std::endl;
  gcore::EnvList l;
  gcore::ForEachInEnv("PATH", l);
  for (size_t i=0; i<l.size(); ++i) {
    std::cout << "  \"" << l[i] << "\"" << std::endl;
  }
  
  return 0;
}

