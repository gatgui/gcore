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
#include <gcore/platform.h>

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
   _mHandle = dlopen(name.c_str(), RTLD_LAZY|RTLD_LOCAL);
#else   //_WIN32
   _mHandle = LoadLibraryEx(name.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
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

