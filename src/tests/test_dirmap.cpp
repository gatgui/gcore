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

int main(int argc, char **argv)
{
   if (argc <= 1)
   {
      gcore::dirmap::AddMapping("z:/home", "/Users");
      gcore::dirmap::AddMapping("z:/work", "/Work");
   }
   else
   {
      gcore::dirmap::ReadMappingsFromFile(gcore::Path(argv[1]));
   }
   
   gcore::String path0 = "/Users/gatgui/dev/temp";
   gcore::String path1 = "/Work/projectA/data/";
   gcore::String path2 = "z:\\home\\gatgui/.preferences";
   gcore::String path3 = "z:/WORK/ProjectA/scenes\\";
   
   std::cout << path0 << " -> " << gcore::dirmap::Map(path0) << std::endl;
   std::cout << path1 << " -> " << gcore::dirmap::Map(path1) << std::endl;
   std::cout << path2 << " -> " << gcore::dirmap::Map(path2) << std::endl;
   std::cout << path3 << " -> " << gcore::dirmap::Map(path3) << std::endl;
   
   
   gcore::Path npath("/Users/gatgui/music");
   std::cout << npath.fullname() << std::endl;
   
   gcore::Path wpath("z:\\home\\GatGui/Music");
   std::cout << wpath.fullname() << std::endl;
   
   return 0;
}
