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

#include "log.h"

PyObject *gPyLogFunc = NULL;
gcore::Log::OutputFunc gLogFunc;

static void PyLogOutputFunc(const char *msg)
{
   if (gPyLogFunc == NULL)
   {
      throw std::runtime_error("gcore.Log: Null output callback");
   }
   PyObject *rv = PyObject_CallFunction(gPyLogFunc, (char*)"s", msg);
   if (rv == NULL)
   {
      PyErr_Clear();
      throw std::runtime_error("gcore.Log: callback failed");
   }
   Py_DECREF(rv);
}

void PyLog_SetOutputFunc(PyObject *outputFunc)
{
   if (outputFunc == NULL)
   {
      throw std::runtime_error("gcore.Log.SetOutputFunc: Null object passed");
   }
   
   if (PyCallable_Check(outputFunc) == 0)
   {
      throw std::runtime_error("gcore.Log.SetOutputFunc: Object is not callable");
   }
   
   if (gPyLogFunc != NULL)
   {
      if (outputFunc != gPyLogFunc)
      {
         Py_DECREF(gPyLogFunc);
      }
      else
      {
         return;
      }
   }
   
   gPyLogFunc = outputFunc;
   Py_INCREF(gPyLogFunc);
   
   if (gLogFunc == 0)
   {
      gcore::Bind(PyLogOutputFunc, gLogFunc);
      gcore::Log::SetOutputFunc(gLogFunc);
   }
}

LogOutputFunc::LogOutputFunc()
   : mPyFunc(0)
{
   
}

LogOutputFunc::LogOutputFunc(const LogOutputFunc &rhs)
   : mPyFunc(rhs.mPyFunc)
{
   Py_XINCREF(mPyFunc);
   gcore::Bind(this, METHOD(LogOutputFunc, call), mFunctor);
}

LogOutputFunc::~LogOutputFunc()
{
   Py_XDECREF(mPyFunc);
}

LogOutputFunc& LogOutputFunc::operator=(const LogOutputFunc &rhs)
{
   if (this != &rhs)
   {
      Py_XDECREF(mPyFunc);
      mPyFunc = rhs.mPyFunc;
      Py_XINCREF(mPyFunc);
   }
   return *this;
}

void LogOutputFunc::setPyFunc(PyObject *obj)
{
   Py_XDECREF(mPyFunc);
   mPyFunc = obj;
   Py_XINCREF(mPyFunc);
}

void LogOutputFunc::call(const char *msg)
{
   if (!mPyFunc)
   {
      return;
   }
   
   PyObject *args = Py_BuildValue("(s)", msg);
   PyObject *rv = PyObject_CallObject(mPyFunc, args);
   Py_XDECREF(args);
   Py_XDECREF(rv);
}

void LogOutputFunc::assign(gcore::Log &l)
{
   l.setOutputFunc(mFunctor);
}


