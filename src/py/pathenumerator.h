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

#ifndef __gcorepy_pathenumerator_h__
#define __gcorepy_pathenumerator_h__

#include "Python.h"
#include <gcore/path.h>

class PathEnumerator
{
public:
   
   PathEnumerator();
   PathEnumerator(PyObject *func);
   PathEnumerator(PathEnumerator &rhs);
   ~PathEnumerator();
   
   PathEnumerator& operator=(const PathEnumerator &rhs);
   
   void setPyFunc(PyObject *func);

   bool visit(const gcore::Path &path);
   void apply(const gcore::Path &path, bool recurse=false, unsigned short flags=gcore::Path::FE_ALL);
   
private:
   
   PyObject *mPyFunc;
   PyObject *mPyPath;
   PyObject *mPyArgs;
   gcore::Path::ForEachFunc mFunctor;
};

#endif
