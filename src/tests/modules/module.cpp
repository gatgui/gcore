/*

Copyright (C) 2009  Gaetan Guidet

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

#ifdef WIN32
#  define DLLAPI extern "C" __declspec(dllexport)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
DLLAPI BOOL __stdcall DllMain(HINSTANCE /*hInst*/, DWORD dwReason, LPVOID /*pvReseved*/)
{
  switch(dwReason)
  {
    //called by the process at load time or LoadLibrary [pvReserved NULL when LoadLibrary]
    case DLL_PROCESS_ATTACH:
    //called by the process when finished / FreeLibrary / load failed [pvReserved NULL when FreeLibrary]
    case DLL_PROCESS_DETACH:
    //called by new thread launched by the process
    case DLL_THREAD_ATTACH:
    //called when thread exit cleanly
    case DLL_THREAD_DETACH:
    default:
      break;
  }
  return TRUE;
}
#else  //WIN32
#  define DLLAPI extern "C"
#endif //WIN32

#include <string>
#include <iostream>
using namespace std;

DLLAPI void initialize()
{
  cerr << "Module initialize" << endl;
}

DLLAPI void reset()
{
  cerr << "Module reset" << endl;
}

DLLAPI void update()
{
  cerr << "Module update" << endl;
}

DLLAPI bool isLoaded()
{
  cerr << "Module isLoaded" << endl;
  return true;
}

DLLAPI bool load(const std::string &str)
{
  cerr << "Module load: " << str << endl;
  return true;
}

//-----------------------------------------------------------------------------

