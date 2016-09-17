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

// ---

void AddMapping(const String &wpath, const String &npath)
{
   if (wpath.length() == 0 && npath.length() == 0)
   {
      return;
   }
   
   String wpath2(wpath);
   
   wpath2.tolower().replace('\\', '/');
   
   gsNix2Win[npath] = wpath2;
   gsWin2Nix[wpath2] = npath;
}

void RemoveMapping(const String &wpath, const String &npath)
{
   StringDict::iterator it;
   
   String wpath2(wpath);
   wpath2.tolower().replace('\\', '/');
   
   it = gsWin2Nix.find(wpath2);
   if (it != gsWin2Nix.end())
   {
      gsWin2Nix.erase(it);
   }
   
   it = gsNix2Win.find(npath);
   if (it != gsNix2Win.end())
   {
      gsNix2Win.erase(it);
   }
}

String Map(const String &path)
{
   String lookuppath(path);
   StringDict *lookupmap;
   
   lookuppath.replace('\\', '/');
   
#ifdef _WIN32
   
   if (!IsWindowsPath(path))
   {
      lookupmap = &gsNix2Win;
   }
   else
   {
      return path;
   }
   
#else

   if (IsWindowsPath(path))
   {
      lookuppath.tolower();
      lookupmap = &gsWin2Nix;
   }
   else
   {
      return path;
   }

#endif
   
   String bestpath;
   
   StringDict::iterator it = lookupmap->begin();
   while (it != lookupmap->end())
   {
      if (lookuppath.startswith(it->first))
      {
         if (it->first.length() > bestpath.length())
         {
            bestpath = it->first;
         }
      }
      ++it;
   }
   
   if (bestpath.length() > 0)
   {
      lookuppath = (*lookupmap)[bestpath];
      lookuppath += path.substr(bestpath.length());
      return lookuppath;
   }
   else
   {
      return path;
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
            
            bool wp0 = IsWindowsPath(parts[0]);
            bool wp1 = IsWindowsPath(parts[1]);
            
            if (wp0 == wp1)
            {
               continue;
            }
            
            if (wp0)
            {
               AddMapping(parts[0], parts[1]);
            }
            else
            {
               AddMapping(parts[1], parts[0]);
            }
            
         }
         
         std::getline(is, line);
      }
   }
}

} // dirmap

} // gcore

