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

#include <gcore/dirmap.h>
#include <gcore/rex.h>

static bool IsWindowsPath(const std::string &p) {
  static gcore::Rex wpe(RAW("^[A-Za-z]:[/\\]"));
  return wpe.match(p);
}

namespace gcore {

StringDict Dirmap::msNix2Win;
StringDict Dirmap::msWin2Nix;

void Dirmap::AddMapping(const String &from, const String &to) {
  if (from.length() == 0 && to.length() == 0) {
    return;
  }
  
  if (IsWindowsPath(from)) {
    String from2(from);
    from2.tolower().replace('\\', '/');
    if (IsWindowsPath(to)) {
      // remap win -> win
      msNix2Win[from2] = to;
      // std::cerr << "Add (win/win) '" << from2 << "' -> '" << to << "' to msNix2Win" << std::endl;
    } else {
      msWin2Nix[from2] = to;
      msNix2Win[to] = from;
      // std::cerr << "Add '" << from2 << "' -> '" << to << "' to msWin2Nix" << std::endl;
      // std::cerr << "Add '" << to << "' -> '" << from << "' to msNix2Win" << std::endl;
    }
  } else {
    if (!IsWindowsPath(to)) {
      // remap nix -> nix
      msWin2Nix[from] = to;
      // std::cerr << "Add (nix/nix)'" << from << "' -> '" << to << "' to msWin2Nix" << std::endl;
    } else {
      String to2(to);
      to2.tolower().replace('\\', '/');
      msWin2Nix[to2] = from;
      msNix2Win[from] = to;
      // std::cerr << "Add '" << to2 << "' -> '" << from << "' to msWin2Nix" << std::endl;
      // std::cerr << "Add '" << from << "' -> '" << to << "' to msNix2Win" << std::endl;
    }
  }
}

void Dirmap::RemoveMapping(const String &from, const String &to) {
  StringDict::iterator it;

  String from2(from);
  String to2(to);

  if (IsWindowsPath(from)) {
    from2.tolower().replace('\\', '/');
  }
  it = msWin2Nix.find(from2);
  if (it != msWin2Nix.end()) {
    msWin2Nix.erase(it);
  }
  it = msNix2Win.find(from2);
  if (it != msNix2Win.end()) {
    msNix2Win.erase(it);
  }

  if (IsWindowsPath(to)) {
    to2.tolower().replace('\\', '/');
  }
  it = msWin2Nix.find(from2);
  if (it != msWin2Nix.end()) {
    msWin2Nix.erase(it);
  }
  it = msNix2Win.find(from2);
  if (it != msNix2Win.end()) {
    msNix2Win.erase(it);
  }
}

String Dirmap::_Map(const String &path, StringDict *forcelookup) {
  
  String outpath(path);
  StringDict *lookupmap = forcelookup;

  bool winpath = IsWindowsPath(path);

  if (!forcelookup) {

#ifdef _WIN32
  
    // std::cerr << "Map: Use msNix2Win" << std::endl;
    if (!winpath) {
      outpath = _Map(path, &msWin2Nix);
      winpath = IsWindowsPath(outpath);
    }
    lookupmap = &msNix2Win;

#else

    // std::cerr << "Map: Use msWin2Nix" << std::endl;
    if (winpath) {
      outpath = _Map(path, &msNix2Win);
      winpath = IsWindowsPath(outpath);
    }
    lookupmap = &msWin2Nix;

#endif
  }

  String lookuppath(outpath);
  if (winpath) {
    lookuppath.tolower().replace('\\', '/');
  }

  String bestpath;

  StringDict::iterator it = lookupmap->begin();
  while (it != lookupmap->end()) {
    // std::cerr << "Map: Check against '" << it->first << "'..." << std::endl;
    if (lookuppath.startswith(it->first)) {
      // std::cerr << "Map: Matched" << std::endl;
      if (it->first.length() > bestpath.length()) {
        // std::cerr << "Map: Longest Match" << std::endl;
        bestpath = it->first;
      }
    }
    ++it;
  }

  if (bestpath.length() > 0) {
    lookuppath = (*lookupmap)[bestpath];
    lookuppath += outpath.substr(bestpath.length());
#ifndef _WIN32
    lookuppath.replace('\\', '/');
#endif
    return lookuppath;
  } else {
    return outpath;
  }
}

String Dirmap::Map(const String &path) {
  return _Map(path, 0);
}

void Dirmap::WriteMappingsToFile(const Path &mapfile) {
  std::ofstream os(mapfile.fullname().c_str());
  
  StringDict::iterator it, it2;
  
  for (it = msNix2Win.begin(); it != msNix2Win.end(); ++it) {
    os << it->first << " = " << it->second << std::endl;
  }
  
  for (it = msWin2Nix.begin(); it != msWin2Nix.end(); ++it) {
    if (!IsWindowsPath(it->first) && !IsWindowsPath(it->second)) {
      // nix -> nix
      os << it->first << " = " << it->second << std::endl;
    } else {
      // win -> nix
      it2 = msNix2Win.find(it->second);
      if (it2 != msNix2Win.end()) {
        String tmp(it2->second);
        tmp.tolower().replace('\\', '/');
        if (tmp == it->first) {
          continue;
        }
      }
      os << it->first << " = " << it->second << std::endl;
    }
  }
}

void Dirmap::ReadMappingsFromFile(const Path &mapfile) {
  
  if (mapfile.isFile()) {
    std::ifstream is(mapfile.fullname().c_str());
    
    String line;
    StringList parts;
    
    std::getline(is, line);
    
    while (is.good()) {
      
      if (line.split('=', parts) == 2) {
        
        parts[0].strip();
        parts[1].strip();
        
        AddMapping(parts[0], parts[1]);
      }
      
      std::getline(is, line);
    }
  }
}

}

