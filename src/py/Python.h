/*
MIT License

Copyright (c) 2010 Gaetan Guidet

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

