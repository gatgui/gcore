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


