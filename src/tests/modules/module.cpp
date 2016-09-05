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

#include <cstring>
#include <string>
#include <iostream>

#ifdef WIN32
#  define DLLAPI extern "C" __declspec(dllexport)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
DLLAPI BOOL __stdcall DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID /*pvReseved*/)
{
  switch(dwReason)
  {
    //called by the process at load time or LoadLibrary [pvReserved NULL when LoadLibrary]
    case DLL_PROCESS_ATTACH:
      {
        static char buffer[1024]; // 1k should be enough no?
        DWORD len = GetModuleFileName(hInst, buffer, 1024);
        // returned length doesn't include trailing '\0'
        if (len > 0 && len < 1024)
        {
          std::cerr << "Module path: " << buffer << std::endl;
        }
        else if (len == 1024 && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
          std::cerr << "Module path is longer that 1023 characters" << std::endl;
          // either failed or buffer too small
        }
        else
        {
          std::cerr << "Failed to retrieve module path" << std::endl;
        }
      }
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
#  include <dlfcn.h>
#  include <cstdlib>
#  define DLLAPI extern "C" __attribute__ ((visibility ("default")))
__attribute__((constructor))
void moduleinit()
{
  Dl_info dli;

  memset(&dli, 0, sizeof(Dl_info));
  if (dladdr((const void*)moduleinit, &dli) != 0)
  {
    std::cerr << "Module path: " << dli.dli_fname << std::endl;
  }
  else
  {
    std::cerr << "Failed to retrieve module path" << std::endl;
  }
}
#endif //WIN32

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

