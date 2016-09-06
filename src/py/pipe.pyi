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
   
   def getName(self):
      return self._cobj.getName().c_str()
   
   def isOwned(self):
      return self._cobj.isOwned()
   
   def canRead(self):
      return self._cobj.canRead()
   
   def canWrite(self):
      return self._cobj.canWrite()
   
   def create(self):
      return self._cobj.create()
   
   def open(self, path):
      return self._cobj.open(gcore.String(<char*?>path))
   
   def close(self):
      self._cobj.close()
   
   def closeRead(self):
      self._cobj.closeRead()
   
   def closeWrite(self):
      self._cobj.closeWrite()
   
   def read(self, retryOnInterrupt=False):
      cdef gcore.String out
      rlen = self._cobj.read(out, retryOnInterrupt)
      return (rlen, out.c_str())
   
   def write(self, msg):
      return self._cobj.write(gcore.String(<char*?>msg))
   
