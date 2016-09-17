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

#include "pathenumerator.h"
#include <gcore/all.h>
#include "log.h"
#ifdef DL_IMPORT
#  undef DL_IMPORT
#  define DL_IMPORT DL_EXPORT
#endif
#include "_gcore.h"


PathEnumerator::PathEnumerator()
   : mPyFunc(0), mPyPath(0), mPyArgs(0)
{
   gcore::Bind(this, METHOD(PathEnumerator, visit), mFunctor);
}

PathEnumerator::PathEnumerator(PyObject *func)
   : mPyFunc(func), mPyPath(0), mPyArgs(0)
{
   if (mPyFunc)
   {
      Py_INCREF(mPyFunc);
      mPyPath = _PyObject_New(&PyPathType);
      if (mPyPath)
      {
         mPyArgs = Py_BuildValue("O", mPyPath);
      }
   }
   gcore::Bind(this, METHOD(PathEnumerator, visit), mFunctor);
}

PathEnumerator::PathEnumerator(PathEnumerator &rhs)
   : mPyFunc(rhs.mPyFunc), mPyPath(0), mPyArgs(0)
{
   if (mPyFunc)
   {
      Py_INCREF(mPyFunc);
      mPyPath = _PyObject_New(&PyPathType);
      if (mPyPath)
      {
         mPyArgs = Py_BuildValue("O", mPyPath);
      }
   }
   gcore::Bind(this, METHOD(PathEnumerator, visit), mFunctor);
}

PathEnumerator::~PathEnumerator()
{
   Py_XDECREF(mPyFunc);
   Py_XDECREF(mPyArgs);
   Py_XDECREF(mPyPath);
}

PathEnumerator& PathEnumerator::operator=(const PathEnumerator &rhs)
{
   if (this != &rhs)
   {
      Py_XDECREF(mPyFunc);
      mPyFunc = rhs.mPyFunc;
      Py_XINCREF(mPyFunc);
   }
   return *this;
}

void PathEnumerator::setPyFunc(PyObject *func)
{
   Py_XDECREF(mPyFunc);
   mPyFunc = func;
   Py_XINCREF(mPyFunc);
}

bool PathEnumerator::visit(const gcore::Path &path)
{
   if (!mPyFunc)
   {
      return false;
   }
   
   if (!mPyPath)
   {
      Py_XDECREF(mPyArgs);
      mPyArgs = NULL;
      
      mPyPath = PyObject_CallObject((PyObject*) &PyPathType, NULL);
      if (!mPyPath)
      {
         return false;
      }
   }
   
   if (!mPyArgs)
   {
      mPyArgs = Py_BuildValue("(O)", mPyPath);
      if (!mPyArgs)
      {
         return false;
      }
   }
   
   PyPath *pyp = (PyPath*) mPyPath;
   *(pyp->_cobj) = path;
   
   PyObject *rv = PyObject_CallObject(mPyFunc, mPyArgs);
   if (!rv)
   {
      //PyErr_Clear();
      return false;
   }
   else
   {
      bool crv = (rv == Py_True);
      Py_DECREF(rv);
      return crv;
   }
}

void PathEnumerator::apply(const gcore::Path &path, bool recurse, unsigned short flags)
{
   path.forEach(mFunctor, recurse, flags);
}


