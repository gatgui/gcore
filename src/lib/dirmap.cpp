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

static bool IsWindowsPath(const String &p)
{
   static Rex wpe(RAW("^[A-Za-z]:[/\\]"));
   return wpe.match(p);
}

static String _Map(const String &path, StringDict *forcelookup)
{
   String outpath(path);
   StringDict *lookupmap = forcelookup;

   bool winpath = IsWindowsPath(path);

   if (!forcelookup)
   {
#ifdef _WIN32
      // std::cerr << "Map: Use gsNix2Win" << std::endl;
      if (!winpath)
      {
         outpath = _Map(path, &gsWin2Nix);
         winpath = IsWindowsPath(outpath);
      }
      lookupmap = &gsNix2Win;
#else
      // std::cerr << "Map: Use gsWin2Nix" << std::endl;
      if (winpath)
      {
         outpath = _Map(path, &gsNix2Win);
         winpath = IsWindowsPath(outpath);
      }
      lookupmap = &gsWin2Nix;
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
      lookuppath = (*lookupmap)[bestpath];
      lookuppath += outpath.substr(bestpath.length());
#ifndef _WIN32
      lookuppath.replace('\\', '/');
#endif
      return lookuppath;
   }
   else
   {
      return outpath;
   }
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
         gsNix2Win[from2] = to;
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
         gsWin2Nix[from] = to;
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

   if (IsWindowsPath(from))
   {
      from2.tolower().replace('\\', '/');
   }
   it = gsWin2Nix.find(from2);
   if (it != gsWin2Nix.end())
   {
      gsWin2Nix.erase(it);
   }
   it = gsNix2Win.find(from2);
   if (it != gsNix2Win.end())
   {
      gsNix2Win.erase(it);
   }

   if (IsWindowsPath(to))
   {
      to2.tolower().replace('\\', '/');
   }
   it = gsWin2Nix.find(to2);
   if (it != gsWin2Nix.end())
   {
      gsWin2Nix.erase(it);
   }
   it = gsNix2Win.find(to2);
   if (it != gsNix2Win.end())
   {
      gsNix2Win.erase(it);
   }
}

String Map(const String &path)
{
   return _Map(path, 0);
}

void WriteMappingsToFile(const Path &mapfile)
{
   std::ofstream os(mapfile.fullname().c_str());

   StringDict::iterator it, it2;
  
   for (it = gsNix2Win.begin(); it != gsNix2Win.end(); ++it)
   {
      os << it->first << " = " << it->second << std::endl;
   }

   for (it = gsWin2Nix.begin(); it != gsWin2Nix.end(); ++it)
   {
      if (!IsWindowsPath(it->first) && !IsWindowsPath(it->second))
      {
         // nix -> nix
         os << it->first << " = " << it->second << std::endl;
      }
      else
      {
         // win -> nix
         it2 = gsNix2Win.find(it->second);
         if (it2 != gsNix2Win.end())
         {
            String tmp(it2->second);
            tmp.tolower().replace('\\', '/');
            if (tmp == it->first)
            {
               continue;
            }
         }
         os << it->first << " = " << it->second << std::endl;
      }
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

