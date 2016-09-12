cimport gcore
from cython.operator cimport dereference as deref

ctypedef public class Pipe [object PyPipe, type PyPipeType]:
   cdef gcore.Pipe *_cobj
   cdef bint _own
   
   def __cinit__(self, *args, **kwargs):
      self._cobj = NULL
      self._own = False
   
   def __init__(self, *args, noalloc=False, **kwargs):
      if noalloc:
         self._cobj = NULL
         self._own = False
         return
      
      if len(args) == 0:
         self._cobj = new gcore.Pipe()
      elif len(args) == 1:
         self._cobj = new gcore.Pipe(deref((<Pipe?> args[0])._cobj))
      else:
         raise Exception("_gcore.Pipe() accepts at most 1 argument")
      
      self._own = True
   
   def __dealloc__(self):
      if self._own and self._cobj != NULL:
         del(self._cobj)
         self._cobj = NULL
   
   def isNamed(self):
      return self._cobj.isNamed()
   
   property name:
      def __get__(self): return self._cobj.name().c_str()
      def __set__(self, v): raise Exception("_gcore.Pipe.name is not settable")
   
   def isOwned(self):
      return self._cobj.isOwned()
   
   def canRead(self):
      return self._cobj.canRead()
   
   def canWrite(self):
      return self._cobj.canWrite()
   
   def create(self):
      cdef gcore.Status stat
      stat = self._cobj.create()
      return stat.succeeded()
   
   def open(self, path):
      cdef gcore.Status stat
      stat = self._cobj.open(gcore.String(<char*?>path))
      return stat.succeeded()
   
   def close(self):
      self._cobj.close()
   
   def closeRead(self):
      self._cobj.closeRead()
   
   def closeWrite(self):
      self._cobj.closeWrite()
   
   def read(self):
      cdef gcore.Status stat
      cdef char tmp[256]
      rlen = self._cobj.read(tmp, 256, &stat)
      if stat.failed():
         print("_gcore.Pipe.read: Cannot read from pipe (%s)" % stat.message())
      return (rlen, tmp)
   
   def write(self, msg):
      cdef gcore.Status stat
      wlen = self._cobj.write(gcore.String(<char*?>msg), &stat)
      if stat.failed():
         print("_gcore.Pipe.write: Cannot write from pipe (%s)" % stat.message())
      return wlen
   
