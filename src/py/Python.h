#ifndef __gcorepy_Python_h__
#define __gcorepy_Python_h__

#include <Python.h>

#ifndef _WIN32
#  undef  PyMODINIT_FUNC
#  define PyMODINIT_FUNC extern "C" __attribute__ ((visibility("default"))) void
#endif

#endif

