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

#ifndef __gcorepy_log_h__
#define __gcorepy_log_h__

#include "Python.h"
#include <gcore/log.h>

void PyLog_SetOutputFunc(PyObject *outputFunc);

class LogOutputFunc
{
public:
   LogOutputFunc();
   LogOutputFunc(const LogOutputFunc &rhs);
   ~LogOutputFunc();
   
   LogOutputFunc& operator=(const LogOutputFunc &rhs);
   
   void setPyFunc(PyObject *obj);
   
   void call(const char *msg);
   void assign(gcore::Log &l);
   
private:
   
   PyObject *mPyFunc;
   gcore::Log::OutputFunc mFunctor; 
};

#endif

