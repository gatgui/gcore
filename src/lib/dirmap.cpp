/*

Copyright (C) 2010~  Gaetan Guidet

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

namespace gcore
{

namespace dirmap
{

static StringDict gsNix2Win;
static StringDict gsWin2Nix;
static StringDict gsNixNix;
//static StringDict gsRevNixNix;
static StringDict gsWinWin;
//static StringDict gsRevWinWin;

static bool IsWindowsPath(const String &p)
{
   static Rex wpe(RAW("^[A-Za-z]:[/\\]"));
   return wpe.match(p);
}

static String _Map(const String &path, bool reverseLookup, StringDict *forcelookup)
{
   String outpath(path);
   StringDict *lookupmap = forcelookup;

   bool winpath = IsWindowsPath(path);
   // gsNix2Win also tracks windows -> windows path conversion
   // gsWin2Nix also tracks unix -> unix path conversion

   if (!forcelookup)
   {
#ifdef _WIN32
      // std::cerr << "Map: Use gsNix2Win" << std::endl;
      if (!reverseLookup)
      {
         // We want to map from unix path to windows path
         outpath = _Map(path, false, winpath ? &gsWinWin : &gsNixNix);
         lookupmap = &gsNix2Win;
      }
      else
      {
         // We want to map from windows path to unix path
         outpath = _Map(path, false, winpath ? &gsWinWin : &gsNixNix);
         lookupmap = &gsWin2Nix;
      }
#else
      // std::cerr << "Map: Use gsWin2Nix" << std::endl;
      if (!reverseLookup)
      {
         // We want to map from windows path to unix path
         outpath = _Map(path, false, winpath ? &gsWinWin : &gsNixNix);
         lookupmap = &gsWin2Nix;
      }
      else
      {
         // We want to map from unix path to windows path
         outpath = _Map(path, false, winpath ? &gsWinWin : &gsNixNix);
         lookupmap = &gsNix2Win;
      }
#endif
   }

   String lookuppath(outpath);
   if (winpath)
   {
      lookuppath.tolower().replace('\\', '/');
   }

   String bestpath;

   StringDict::iterator it = lookupmap->begin();
   while (it != lookupmap->end())
   {
      // std::cerr << "Map: Check against '" << it->first << "'..." << std::endl;
      if (lookuppath.startswith(it->first))
      {
         // std::cerr << "Map: Matched" << std::endl;
         if (it->first.length() > bestpath.length())
         {
            // std::cerr << "Map: Longest Match" << std::endl;
            bestpath = it->first;
         }
      }
      ++it;
   }

   if (bestpath.length() > 0)
   {
      outpath = (*lookupmap)[bestpath] + outpath.substr(bestpath.length());
   }

   outpath.replace('\\', '/');
   return outpath;
}

// ---

void AddMapping(const String &from, const String &to)
{
   if (from.length() == 0 && to.length() == 0)
   {
      return;
   }
  
   if (IsWindowsPath(from))
   {
      String from2(from);
      from2.tolower().replace('\\', '/');
      if (IsWindowsPath(to))
      {
         // remap win -> win
         //gsNix2Win[from2] = to;
         gsWinWin[from2] = to;
         // std::cerr << "Add (win/win) '" << from2 << "' -> '" << to << "' to gsNix2Win" << std::endl;
      }
      else
      {
         gsWin2Nix[from2] = to;
         gsNix2Win[to] = from;
         // std::cerr << "Add '" << from2 << "' -> '" << to << "' to gsWin2Nix" << std::endl;
         // std::cerr << "Add '" << to << "' -> '" << from << "' to gsNix2Win" << std::endl;
      }
   }
   else
   {
      if (!IsWindowsPath(to))
      {
         // remap nix -> nix
         //gsWin2Nix[from] = to;
         gsNixNix[from] = to;
         // std::cerr << "Add (nix/nix)'" << from << "' -> '" << to << "' to gsWin2Nix" << std::endl;
      }
      else
      {
         String to2(to);
         to2.tolower().replace('\\', '/');
         gsWin2Nix[to2] = from;
         gsNix2Win[from] = to;
         // std::cerr << "Add '" << to2 << "' -> '" << from << "' to gsWin2Nix" << std::endl;
         // std::cerr << "Add '" << from << "' -> '" << to << "' to gsNix2Win" << std::endl;
      }
   }
}

void RemoveMapping(const String &from, const String &to)
{
   StringDict::iterator it;

   String from2(from);
   String to2(to);

   bool fwin = IsWindowsPath(from);
   bool twin = IsWindowsPath(to);

   if (fwin)
   {
      from2.tolower().replace('\\', '/');
      if (twin)
      {
         // windows -> windows
         it = gsWinWin.find(from2);
         if (it != gsWinWin.end()) // should check that it->second == to
         {
            gsWinWin.erase(it);
         }
      }
      else
      {
         // windows -> unix
         it = gsWin2Nix.find(from2);
         if (it != gsWin2Nix.end())
         {
            gsWin2Nix.erase(it);
         }
         it = gsNix2Win.find(to);
         if (it != gsNix2Win.end())
         {
            gsNix2Win.erase(it);
         }
      }
   }
   else
   {
      if (twin)
      {
         // unix -> windows
         to2.tolower().replace('\\', '/');
         it = gsNix2Win.find(from);
         if (it != gsNix2Win.end())
         {
            gsNix2Win.erase(it);
         }
         it = gsWin2Nix.find(to2);
         if (it != gsWin2Nix.end())
         {
            gsWin2Nix.erase(it);
         }
      }
      else
      {
         // unix -> unix
         it = gsNixNix.find(from);
         if (it != gsNixNix.end())
         {
            gsNixNix.erase(it);
         }
      }
   }
}

String Map(const String &path, bool reverseLookup)
{
   return _Map(path, reverseLookup, 0);
}

void WriteMappingsToFile(const Path &mapfile)
{
   std::ofstream os(mapfile.fullname().c_str());

   StringDict::iterator it, it2;
   
   for (it = gsNix2Win.begin(); it != gsNix2Win.end(); ++it)
   {
      os << it->first << " = " << it->second << std::endl;
   }
   // don't need to process gsWin2Nix

   for (it = gsNixNix.begin(); it != gsNixNix.end(); ++it)
   {
      os << it->first << " = " << it->second << std::endl;
   }

   for (it = gsWinWin.begin(); it != gsWinWin.end(); ++it)
   {
      os << it->first << " = " << it->second << std::endl;
   }
}

void ReadMappingsFromFile(const Path &mapfile)
{
   if (mapfile.isFile())
   {
      std::ifstream is(mapfile.fullname().c_str());

      String line;
      StringList parts;

      std::getline(is, line);

      while (is.good())
      {
         if (line.split('=', parts) == 2)
         {
           
           parts[0].strip();
           parts[1].strip();
           
           AddMapping(parts[0], parts[1]);
         }

         std::getline(is, line);
      }
   }
}

} // dirmap

} // gcore

