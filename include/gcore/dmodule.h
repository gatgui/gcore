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

#ifndef __gcore_dmodule_h_
#define __gcore_dmodule_h_

#include <gcore/string.h>

namespace gcore {

  class GCORE_API DynamicModule {
    
    public:
      
      DynamicModule();
      DynamicModule(const String &name);
      virtual ~DynamicModule();
      
      bool _opened() const;
      bool _open(const String &name);
      bool _close();
      void* _getSymbol(const String &symbol) const;
      String _getError() const;
      
    private:
      
      void *_mHandle;
      String _mName;
      mutable std::map<String,void*> _mSymbolMap;
  };
}

#ifdef WIN32
# define GCORE_MODULE_API extern "C" __declspec(dllexport)
# define GCORE_MODULE_DUMMY_MAIN \
  GCORE_MODULE_API BOOL __stdcall DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID pvReserved) { \
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
# define GCORE_MODULE_API extern "C" __attribute__ ((visibility ("default")))
# define GCORE_MODULE_DUMMY_MAIN
#endif


#define GCORE_BEGIN_MODULE_INTERFACE(ClassName)\
  class ClassName : public gcore::DynamicModule {\
    public:\
      ClassName() : gcore::DynamicModule() {}\
      ClassName(const gcore::String &name) : gcore::DynamicModule(name) {}\
      virtual ~ClassName() {}

#define GCORE_DEFINE_MODULE_SYMBOL0(Name)\
    public:\
      void Name() {\
        Name##_t ptr = FPTR_CAST(Name##_t, _getSymbol(#Name));\
        if (ptr) {\
          (*ptr)();\
        }\
      }\
    protected:\
      typedef void (*Name##_t)();
  
#define GCORE_DEFINE_MODULE_SYMBOL1(Name,Type1)\
    public:\
      void Name(Type1 p1) {\
        Name##_t ptr = FPTR_CAST(Name##_t, _getSymbol(#Name));\
        if (ptr) {\
          (*ptr)(p1);\
        }\
      }\
    protected:\
      typedef void (*Name##_t)(Type1);
    
#define GCORE_DEFINE_MODULE_SYMBOL2(Name,Type1,Type2)\
    public:\
      void Name(Type1 p1,Type2 p2) {\
        Name##_t ptr = FPTR_CAST(Name##_t, _getSymbol(#Name));\
        if (ptr) {\
          (*ptr)(p1,p2);\
        }\
      }\
    protected:\
      typedef void (*Name##_t)(Type1,Type2);

#define GCORE_DEFINE_MODULE_SYMBOL3(Name,Type1,Type2,Type3)\
    public:\
      void Name(Type1 p1, Type2 p2, Type3 p3) {\
        Name##_t ptr = FPTR_CAST(Name##_t, _getSymbol(#Name));\
        if (ptr) {\
          (*ptr)(p1,p2,p3);\
        }\
      }\
    protected:\
      typedef void (*Name##_t)(Type1,Type2,Type3);

#define GCORE_DEFINE_MODULE_SYMBOL4(Name,Type1,Type2,Type3,Type4)\
    public:\
      void Name(Type1 p1, Type2 p2, Type3 p3, Type4 p4) {\
        Name##_t ptr = FPTR_CAST(Name##_t, _getSymbol(#Name));\
        if (ptr) {\
          (*ptr)(p1,p2,p3,p4);\
        }\
      }\
    protected:\
      typedef void (*Name##_t)(Type1,Type2,Type3,Type4);

#define GCORE_DEFINE_MODULE_SYMBOL0R(RType,Name)\
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
  
#define GCORE_DEFINE_MODULE_SYMBOL1R(RType,Name,Type1)\
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
    
#define GCORE_DEFINE_MODULE_SYMBOL2R(RType,Name,Type1,Type2)\
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

#define GCORE_DEFINE_MODULE_SYMBOL3R(RType,Name,Type1,Type2,Type3)\
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

#define GCORE_DEFINE_MODULE_SYMBOL4R(RType,Name,Type1,Type2,Type3,Type4)\
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

#define GCORE_END_MODULE_INTERFACE \
  };

#endif


