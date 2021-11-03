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
#  define DLLAPI extern "C" __attribute__ ((visibility ("default")))
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

