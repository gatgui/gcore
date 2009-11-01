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
// 4290: throw(ExceptionList) ignore warning
# pragma warning(disable: 4251 4290 4702)
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <map>
#include <vector>
#include <deque>
#include <list>
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdarg>
#include <ctime>
#include <exception>
#include <stdexcept>

/*
#ifdef _WIN32
# ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
# endif
# ifndef _WIN32_WINNT
#   define _WIN32_WINNT WINVER
# endif
# include <windows.h>
#else
# include <sys/wait.h>
# include <sys/time.h>
# include <unistd.h>
# include <errno.h>
# include <dlfcn.h>
# include <dirent.h>
# include <pthread.h>
# include <semaphore.h>
# include <sched.h>
#endif
*/

#if defined(__LP64__) ||\
    defined(_LP64) ||\
    (_MIPS_SZLONG == 64) ||\
    (__WORDSIZE == 64)
// Native 64 bits long architecture
typedef unsigned long UInt64;
typedef long Int64;
#elif defined(_MSC_VER) ||\
      (defined(__BCPLUSPLUS__) && __BORLANDC__ > 0x500) ||\
      defined(__WATCOM_INT64__)
// Compiler native 64 bit type
typedef unsigned __int64 UInt64;
typedef __int64 Int64;
#elif defined(__GNUG__) ||\
      defined(__GNUC__) ||\
      defined(__SUNPRO_CC) ||\
      defined(__MWERKS__) ||\
      defined(__SC__) ||\
      defined(_LONGLONG)
// Compiler native 64 bit type
typedef unsigned long long UInt64;
typedef long long Int64;
#else
// No 64 bit support define it to be 32 bit
typedef unsigned long UInt64;
typedef long Int64;
#endif

//#define FPTR_CAST(target_type, ptr_var) ((target_type)(UInt64)(ptr_var))
#define FPTR_CAST(target_type, ptr_var) ((target_type)(size_t)(ptr_var))

#endif
