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


