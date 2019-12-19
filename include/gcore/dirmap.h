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

#ifndef __gcore_dirmap_h_
#define __gcore_dirmap_h_

#include <gcore/string.h>
#include <gcore/path.h>

namespace gcore {
   
  class GCORE_API Dirmap
  {
    private:
      
      static StringDict msWin2Nix;
      static StringDict msNix2Win;
      static String _Map(const String &path, StringDict *lookup);

    public:
      
      static void AddMapping(const String &from, const String &to);
      static void RemoveMapping(const String &from, const String &to);
      static void ReadMappingsFromFile(const Path &mapfile);
      static String Map(const String &path);
  };
  
}

#endif
