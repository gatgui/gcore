#ifndef __gcorepy_log_h__
#define __gcorepy_log_h__

#include <Python.h>
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

