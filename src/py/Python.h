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

