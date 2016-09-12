/*

Copyright (C) 2009, 2010  Gaetan Guidet

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

#include <gcore/dmodule.h>
#include <iostream>

// This describe a dll interface in an object so that calling method
// on this obejct call the dll function
// DLL used this way do not have to be link at compile time
// A possible use is for a plugin architecture
GCORE_BEGIN_MODULE_INTERFACE    ( MyDll                               )
   GCORE_DEFINE_MODULE_SYMBOL0  (       initialize                    )
   GCORE_DEFINE_MODULE_SYMBOL0  (       reset                         )
   GCORE_DEFINE_MODULE_SYMBOL0  (       update                        )
   GCORE_DEFINE_MODULE_SYMBOL0R ( bool, isLoaded                      )
   GCORE_DEFINE_MODULE_SYMBOL1R ( bool, load,      const std::string& )
GCORE_END_MODULE_INTERFACE

int main(int,char**)
{
#if defined(WIN32)
   MyDll dll("testmodule.dll");
#elif defined(__APPLE__)
   MyDll dll("testmodule.bundle");
#else
   MyDll dll("testmodule.so");
#endif

   dll.initialize();

   if (dll.isLoaded())
   {
      dll.load("./conf.cfg");
   }
   else
   {
      std::cerr << "Could not load library" << std::endl;
      std::cerr << dll._getError() << std::endl;
   }

   dll.update();

   dll.reset();

   return 0;
}

