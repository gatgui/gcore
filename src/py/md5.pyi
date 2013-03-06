cimport gcore
from cython.operator cimport dereference as deref

ctypedef public class MD5 [object PyMD5, type PyMD5Type]:
   cdef gcore.MD5 *_cobj
   cdef bint _own
   
   def __cinit__(self, *args, **kwargs):
      self._cobj = NULL
      self._own = False
   
   def __init__(self, *args, noalloc=False, **kwargs):
      if noalloc:
         self._cobj = NULL
         self._own = False
      
      if len(args) == 0:
         self._cobj = new gcore.MD5()
      elif len(args) == 1:
         if type(args[0]) in [str, unicode]:
            self._cobj = new gcore.MD5(gcore.String(<char*>args[0]))
         else:
            self._cobj = new gcore.MD5(deref((<MD5?> args[0])._cobj))
      else:
         raise Exception("_gcore.MD5() accepts at most 1 argument")
      
      self._own = True
   
   def __dealloc__(self):
      if self._own and self._cobj != NULL:
         del(self._cobj)
         self._cobj = NULL
   
   def update(self, s):
      cdef char *buffer = <char*?> s
      cdef int count = len(s)
      self._cobj.update(buffer, count)
   
   def clear(self):
      self._cobj.clear()
   
   def __str__(self):
      return self._cobj.asString().c_str()
