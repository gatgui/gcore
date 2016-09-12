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

#include <iostream>
#include <cstdlib>

int main(int, char **)
{
   char *val = getenv("CUSTOM_ENV");
   std::cout << "Writing stuffs to stdout (CUSTOM_ENV = " << (val ? val : "<undefined>") << ")" << std::endl;
   std::cerr << "Writing stuffs to stderr" << std::endl;

   return 0;
}
  