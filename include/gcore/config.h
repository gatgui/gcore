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

#ifndef __gcore_config_h_
#define __gcore_config_h_

#ifndef GCORE_STATIC
# ifdef _WIN32
#   ifdef GCORE_EXPORTS
#     define GCORE_API __declspec(dllexport)
#   else
#     define GCORE_API __declspec(dllimport)
#   endif
# else
#   define GCORE_API __attribute__((visibility("default")))
# endif
#else
# define GCORE_API
#endif

#ifdef _MSC_VER
// always disable
// 4251: warning on template class not having a dll-interface
// 4275: non-dll interface class used as base for dll-interface class
# pragma warning(disable: 4251 4275 4702)
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cassert>
#include <cstring>
#include <cstdarg>
#include <cerrno>
#include <ctime>
#include <map>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <exception>
#include <stdexcept>

#define FPTR_CAST(target_type, ptr_var) ((target_type)(size_t)(ptr_var))

namespace gcore  {
  
#if defined(__LP64__) || defined(_LP64) || (_MIPS_SZLONG == 64) || (__WORDSIZE == 64)
# define NATIVE_64BIT_LONG 1
  // Native 64 bits long architecture
  typedef unsigned long UInt64;
  typedef long Int64;
#elif defined(_MSC_VER) || (defined(__BCPLUSPLUS__) && __BORLANDC__ > 0x500) || defined(__WATCOM_INT64__)
# define NATIVE_64BIT_LONG 0
  // Compiler 64 bit type
  typedef unsigned __int64 UInt64;
  typedef __int64 Int64;
#elif defined(__GNUG__) || defined(__GNUC__) || defined(__SUNPRO_CC) || defined(__MWERKS__) || defined(__SC__) || defined(_LONGLONG)
# define NATIVE_64BIT_LONG 0  
  // Compiler 64 bit type
  typedef unsigned long long UInt64;
  typedef long long Int64;
#else
# define NATIVE_64BIT_LONG 0
  // No 64 bit support. Define it to be 32 bit
  typedef unsigned long UInt64;
  typedef long Int64;
#endif

}

#endif
