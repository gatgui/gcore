#include "log.h"

PyObject *gPyLogFunc = NULL;
gcore::Log::OutputFunc gLogFunc;

static void PyLogOutputFunc(const char *msg)
{
   if (gPyLogFunc == NULL)
   {
      throw std::runtime_error("mzenv.Log: Null output callback");
   }
   PyObject *rv = PyObject_CallFunction(gPyLogFunc, (char*)"s", msg);
   if (rv == NULL)
   {
      PyErr_Clear();
      throw std::runtime_error("mzenv.Log: callback failed");
   }
   Py_DECREF(rv);
}

void PyLog_SetOutputFunc(PyObject *outputFunc)
{
   if (outputFunc == NULL)
   {
      throw std::runtime_error("mzenv.Log.SetOutputFunc: Null object passed");
   }
   
   if (PyCallable_Check(outputFunc) == 0)
   {
      throw std::runtime_error("mzenv.Log.SetOutputFunc: Object is not callable");
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


