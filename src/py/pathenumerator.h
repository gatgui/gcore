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
   void apply(const gcore::Path &path, bool recurse=false, unsigned short flags=gcore::Path::ET_ALL);
   
private:
   
   PyObject *mPyFunc;
   PyObject *mPyPath;
   PyObject *mPyArgs;
   gcore::Path::EachFunc mFunctor;
};

#endif
