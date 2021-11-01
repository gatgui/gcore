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
#include <gcore/platform.h>
#include <gcore/unicode.h>

namespace gcore
{

DynamicModule::DynamicModule()
   : _mHandle(0)
   , _mName("") 
{
}

DynamicModule::DynamicModule(const String &name)
   : _mHandle(0)
   , _mName(name) 
{
   _open(name);
}

DynamicModule::~DynamicModule()
{
   if (_opened())
   {
      _close();
   }
}

bool DynamicModule::_opened() const
{
   return (_mHandle != 0);
}

bool DynamicModule::_open(const String &name)
{
   if (_opened())
   {
      if (name == _mName)
      {
         return true;
      }
      _close();
   }
#ifndef _WIN32
   // RTLD_LAZY / RTLD_NOW (check all exported symbols at load time)
   // RTLD_GLOBAL / RTLD_LOCAL (symbol can be access by using RTLD_DEFAULT or RTLD_NEXT handle, or only through dlopen handle)
   std::string lname;
   if (UTF8ToLocale(name.c_str(), lname))
   {
      _mHandle = dlopen(lname.c_str(), RTLD_LAZY|RTLD_LOCAL);
   }
#else   //_WIN32
   std::wstring wn;
   DecodeUTF8(name.c_str(), wn);
   _mHandle = LoadLibraryExW(wn.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#endif  //_WIN32
   return (_mHandle != 0);
}

bool DynamicModule::_close()
{
   bool r;
#ifndef _WIN32
   r = (dlclose(_mHandle) == 0);
#else  //_WIN32
   r = (FreeLibrary((HMODULE)_mHandle) == TRUE);
#endif //_WIN32
   _mHandle = 0;
   _mSymbolMap.clear();
   return r;
}

void* DynamicModule::_getSymbol(const String &symbol) const
{
   if (_opened())
   {
      std::map<String, void*>::iterator it = _mSymbolMap.find(symbol);
      if (it != _mSymbolMap.end())
      {
         return (*it).second;
      }
      void *ptr = 0;
#ifndef _WIN32
      ptr = dlsym(_mHandle, symbol.c_str());
#else   //_WIN32
      ptr = (void*)GetProcAddress((HMODULE)_mHandle, symbol.c_str());
#endif  //_WIN32
      if (ptr)
      {
         _mSymbolMap[symbol] = ptr;
      }
      return ptr;
   }
   return 0;
}

String DynamicModule::_getError() const
{
#ifndef _WIN32
   return dlerror();
#else   //_WIN32
   DWORD dw = GetLastError();
   char buffer[512];
   FormatMessage(
      FORMAT_MESSAGE_FROM_SYSTEM,
      NULL, dw, MAKELANGID(LANG_ENGLISH,SUBLANG_DEFAULT),
      (LPTSTR)&buffer, sizeof(buffer), NULL);
   return buffer;
#endif  //_WIN32
}

} // gcore

