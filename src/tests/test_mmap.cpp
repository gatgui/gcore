/*

Copyright (C) 2009~  Gaetan Guidet

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

#include <gcore/path.h>
#include <gcore/argparser.h>

int main(int argc, char **argv)
{
   gcore::FlagDesc flags[] = {ACCEPTS_NOFLAG_ARGUMENTS(1)};
   gcore::ArgParser args(flags, 1);
   
   gcore::Status stat = args.parse(argc-1, argv+1);
   if (!stat)
   {
      std::cerr << stat << std::endl;
      return 1;
   }
   
   gcore::String str;
   args.getArgument(0, str);
   
   std::cout << "Read file '" << str << "'" << std::endl;
   
   gcore::Path path(str);
   
   if (!path.isFile())
   {
      std::cerr << "Not a file '" << str << "'" << std::endl;
      return 1;
   }
   
   std::cout << "Page size: " << gcore::MemoryMappedFile::PageSize() << std::endl;
   
   gcore::MemoryMappedFile mmf;
   
   std::cout << "Map file to memory" << std::endl;
   
   stat = mmf.open(path, gcore::MemoryMappedFile::READ);
   if (!stat)
   {
      std::cerr << stat << std::endl;
      return 1;
   }
   
   std::cout << "Open successfully: size = " << mmf.size() << " [file size = " << path.fileSize() << ", map size: " << mmf.mappedSize() << "]" << std::endl;
   
   unsigned char *bytes = (unsigned char*) mmf.data();
   for (size_t i=0; i<mmf.size(); ++i)
   {
      std::cout << bytes[i];
   }
   
   std::cout << std::endl;
   
   return 0;
}
