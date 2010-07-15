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
#include <gcore/regexp.h>

static bool IsWindowsPath(const std::string &p) {
  static gcore::Regexp wpe(IEC("^[A-Za-z]:[/\\]"));
  return wpe.match(p);
}

namespace gcore {

StringDict Dirmap::msNix2Win;
StringDict Dirmap::msWin2Nix;

void Dirmap::AddMapping(const String &wpath, const String &npath) {
  if (wpath.length() == 0 && npath.length() == 0) {
    return;
  }
  
  String wpath2(wpath);
  
  wpath2.tolower().replace('\\', '/');
  
  msNix2Win[npath] = wpath2;
  msWin2Nix[wpath2] = npath;
}

void Dirmap::RemoveMapping(const String &wpath, const String &npath) {
  StringDict::iterator it;
  
  String wpath2(wpath);
  wpath2.tolower().replace('\\', '/');
  
  it = msWin2Nix.find(wpath2);
  if (it != msWin2Nix.end()) {
    msWin2Nix.erase(it);
  }
  
  it = msNix2Win.find(npath);
  if (it != msNix2Win.end()) {
    msNix2Win.erase(it);
  }
}

String Dirmap::Map(const String &path) {
  
  String lookuppath(path);
  StringDict *lookupmap;
  
#ifdef _WIN32
  
  if (!IsWindowsPath(path)) {
    lookupmap = &msNix2Win;
  } else {
    return path;
  }
  
#else

  if (IsWindowsPath(path)) {
    lookuppath.tolower().replace('\\', '/');
    lookupmap = &msWin2Nix;
  } else {
    return path;
  }

#endif
  
  String bestpath;
  
  StringDict::iterator it = lookupmap->begin();
  while (it != lookupmap->end()) {
    if (lookuppath.startswith(it->first)) {
      if (it->first.length() > bestpath.length()) {
        bestpath = it->first;
      }
    }
    ++it;
  }
  
  if (bestpath.length() > 0) {
    lookuppath = (*lookupmap)[bestpath];
    lookuppath += path.substr(bestpath.length());
#ifndef _WIN32
    lookuppath.replace('\\', '/');
#endif
    return lookuppath;
  } else {
    return path;
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
        
        bool wp0 = IsWindowsPath(parts[0]);
        bool wp1 = IsWindowsPath(parts[1]);
        
        if (wp0 == wp1) {
          continue;
        }
        
        if (wp0) {
          AddMapping(parts[0], parts[1]);
        } else {
          AddMapping(parts[1], parts[0]);
        }
        
      }
      
      std::getline(is, line);
    }
  }
}

}

