/*
MIT License

Copyright (c) 2009 Gaetan Guidet

This file is part of gcore.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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

