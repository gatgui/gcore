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

#ifndef __gcore_dmodule_h_
#define __gcore_dmodule_h_

#include <gcore/config.h>

namespace gcore {

  class GCORE_API DynamicModule {
    
    public:
      
      DynamicModule();
      DynamicModule(const std::string &name);
      virtual ~DynamicModule();
      
      bool _opened() const;
      bool _open(const std::string &name);
      bool _close();
      void* _getSymbol(const std::string &symbol) const;
      std::string _getError() const;
      
    private:
      
      void *_mHandle;
      std::string _mName;
      mutable std::map<std::string,void*> _mSymbolMap;
  };
}

#ifdef WIN32
# define MODULE_API extern "C" __declspec(dllexport)
# define MODULE_DUMMY_MAIN \
  MODULE_API BOOL __stdcall DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID pvReserved) { \
    switch(dwReason) {\
      case DLL_PROCESS_ATTACH: \
      case DLL_PROCESS_DETACH: \
      case DLL_THREAD_ATTACH: \
      case DLL_THREAD_DETACH: \
      default: \
        break; \
    } \
    return TRUE; \
  }
#else
# define MODULE_API extern "C"
# define MODULE_DUMMY_MAIN
#endif


#define BEGIN_MODULE_INTERFACE(ClassName)\
  class ClassName : public gcore::DynamicModule {\
    public:\
      ClassName():gcore::DynamicModule(){}\
      ClassName(const std::string &name):gcore::DynamicModule(name){}\
      virtual ~ClassName(){}

#define END_MODULE_INTERFACE \
  };

#define DEFINE_MODULE_SYMBOL0(Name)\
  public:\
    void Name() {\
      Name##_t ptr = FPTR_CAST(Name##_t, _getSymbol(#Name));\
      if (ptr) {\
        (*ptr)();\
      }\
    }\
  protected:\
    typedef void (*Name##_t)();
  
#define DEFINE_MODULE_SYMBOL1(Name,Type1)\
  public:\
    void Name(Type1 p1) {\
      Name##_t ptr = FPTR_CAST(Name##_t, _getSymbol(#Name));\
      if (ptr) {\
        (*ptr)(p1);\
      }\
    }\
  protected:\
    typedef void (*Name##_t)(Type1);
    
#define DEFINE_MODULE_SYMBOL2(Name,Type1,Type2)\
  public:\
    void Name(Type1 p1,Type2 p2) {\
      Name##_t ptr = FPTR_CAST(Name##_t, _getSymbol(#Name));\
      if (ptr) {\
        (*ptr)(p1,p2);\
      }\
    }\
  protected:\
    typedef void (*Name##_t)(Type1,Type2);

#define DEFINE_MODULE_SYMBOL3(Name,Type1,Type2,Type3)\
  public:\
    void Name(Type1 p1, Type2 p2, Type3 p3) {\
      Name##_t ptr = FPTR_CAST(Name##_t, _getSymbol(#Name));\
      if (ptr) {\
        (*ptr)(p1,p2,p3);\
      }\
    }\
  protected:\
    typedef void (*Name##_t)(Type1,Type2,Type3);

#define DEFINE_MODULE_SYMBOL4(Name,Type1,Type2,Type3,Type4)\
  public:\
    void Name(Type1 p1, Type2 p2, Type3 p3, Type4 p4) {\
      Name##_t ptr = FPTR_CAST(Name##_t, _getSymbol(#Name));\
      if (ptr) {\
        (*ptr)(p1,p2,p3,p4);\
      }\
    }\
  protected:\
    typedef void (*Name##_t)(Type1,Type2,Type3,Type4);

#define DEFINE_MODULE_SYMBOL0R(RType,Name)\
  public:\
    RType Name() {\
      Name##_t ptr = FPTR_CAST(Name##_t, _getSymbol(#Name));\
      if (ptr) {\
        return (*ptr)();\
      }\
      return (RType)0;\
    }\
  protected:\
    typedef RType (*Name##_t)();
  
#define DEFINE_MODULE_SYMBOL1R(RType,Name,Type1)\
  public:\
    RType Name(Type1 p1) {\
      Name##_t ptr = FPTR_CAST(Name##_t, _getSymbol(#Name));\
      if (ptr) {\
        return (*ptr)(p1);\
      }\
      return (RType)0;\
    }\
  protected:\
    typedef RType (*Name##_t)(Type1);
    
#define DEFINE_MODULE_SYMBOL2R(RType,Name,Type1,Type2)\
  public:\
    RType Name(Type1 p1,Type2 p2) {\
      Name##_t ptr = FPTR_CAST(Name##_t, _getSymbol(#Name));\
      if (ptr) {\
        return (*ptr)(p1,p2);\
      }\
      return (RType)0;\
    }\
  protected:\
    typedef RType (*Name##_t)(Type1,Type2);

#define DEFINE_MODULE_SYMBOL3R(RType,Name,Type1,Type2,Type3)\
  public:\
    RType Name(Type1 p1, Type2 p2, Type3 p3) {\
      Name##_t ptr = FPTR_CAST(Name##_t, _getSymbol(#Name));\
      if (ptr) {\
        return (*ptr)(p1,p2,p3);\
      }\
      return (RType)0;\
    }\
  protected:\
    typedef RType (*Name##_t)(Type1,Type2,Type3);

#define DEFINE_MODULE_SYMBOL4R(RType,Name,Type1,Type2,Type3,Type4)\
  public:\
    RType Name(Type1 p1, Type2 p2, Type3 p3, Type4 p4) {\
      Name##_t ptr = FPTR_CAST(Name##_t, _getSymbol(#Name));\
      if (ptr) {\
        return (*ptr)(p1,p2,p3,p4);\
      }\
      return (RType)0;\
    }\
  protected:\
    typedef RType (*Name##_t)(Type1,Type2,Type3,Type4);

// And so on ....
// What about variadic functions ??

#endif


