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

#ifndef __gcore_config_h_
#define __gcore_config_h_

#ifndef GCORE_STATIC
# ifdef GCORE_EXPORTS
#   ifdef _WIN32
#     define GCORE_API __declspec(dllexport)
#   else
#     define GCORE_API
#   endif
# else
#   ifdef _WIN32
#     define GCORE_API __declspec(dllimport)
#   else
#     define GCORE_API
#   endif
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
