/*

Copyright (C) 2010~  Gaetan Guidet

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

#ifndef __gcorepy_Python_h__
#define __gcorepy_Python_h__

#ifdef _MSC_VER
// 4127: conditional expression is constant
// 4100: unreferenced formal parameter
// 4244: conversion from Py_ssize_t to int
// 4267: conversion from size_t to int
// 4800: int forcing value to bool 'true' or 'false'
// 4211: nonstandard extension used: redefined extern to static
//       (see https://connect.microsoft.com/VisualStudio/feedback/details/633988/warning-in-math-h-line-162-re-nonstandard-extensions-used)
// 4510: <unnamed-tag>: default constructor could not be generated
// 4512: <unnamed-tag>: assignment operator could not be generated
// 4610: <unnamed-tag> can never be instanciated - user defined constructor required
# pragma warning(disable: 4127 4100 4244 4267 4800 4211 4510 4512 4610)
# ifdef _USE_MATH_DEFINES
#   undef _USE_MATH_DEFINES
# endif
#endif

#include <Python.h>

#ifndef _WIN32
#  undef  PyMODINIT_FUNC
#  define PyMODINIT_FUNC extern "C" __attribute__ ((visibility("default"))) void
#endif

#endif

