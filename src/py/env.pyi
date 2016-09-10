cimport gcore
from libcpp.map cimport map
from cython.operator cimport dereference as deref
from cython.operator cimport preincrement as pinc

 
ctypedef public class Env [object PyEnv, type PyEnvType]:
   cdef gcore.Env *_cobj
   cdef bint _own
   
   def __cinit__(self, *args, **kwargs):
      self._cobj = NULL
      self._own = False
   
   def __init__(self, noalloc=False):
      if noalloc:
         self._cobj = NULL
         self._own = False
         return
      self._cobj = new gcore.Env()
      self._own = True
   
   def __dealloc__(self):
      if self._own and self._cobj != NULL:
         del self._cobj
         self._cobj = NULL
   
   def push(self):
      self._cobj.push()
   
   def pop(self):
      self._cobj.pop()
   
   def isSet(self, char* key):
      return self._cobj.isSet(gcore.String(key))
   
   def get(self, char* key):
      return self._cobj.get(gcore.String(key)).c_str()
   
   def set(self, *args):
      cdef map[gcore.String, gcore.String] cd
      if len(args) < 2 or len(args) > 3:
         raise Exception("_gcore.Env.set takes 2 to 3 arguments")
      elif len(args) == 2:
         for k, v in args[0].iteritems():
            cd[gcore.String(<char*?>k)] = gcore.String(<char*?>v)
         self._cobj.set(cd, <bint?>args[1])
      else:
         self._cobj.set(gcore.String(<char*?>args[0]), gcore.String(<char*?>args[1]), <bint?>args[2])
   
   def asDict(self):
      cdef map[gcore.String, gcore.String] cd
      cdef map[gcore.String, gcore.String].iterator it
      self._cobj.asDict(cd)
      rv = {}
      it = cd.begin()
      while it != cd.end():
         rv[deref(it).first.c_str()] = deref(it).second.c_str()
         pinc(it)
      return rv
   
   @classmethod
   def Username(klass):
      return gcore.GetUser().c_str()
   
   @classmethod
   def Hostname(klass):
      return gcore.GetHost().c_str()
   
   @classmethod
   def IsSet(klass, char* key):
      return gcore.IsSet(gcore.String(key))
   
   @classmethod
   def Get(klass, char* key):
      return gcore.Get(gcore.String(key)).c_str()
   
   @classmethod
   def Set(klass, *args):
      cdef map[gcore.String, gcore.String] cd
      if len(args) < 2 or len(args) > 3:
         raise Exception("_gcore.Env.Set takes 2 to 3 arguments")
      elif len(args) == 2:
         for k, v in args[0].iteritems():
            cd[gcore.String(<char*?>k)] = gcore.String(<char*?>v)
         gcore.Set(cd, <bint?>args[1])
      else:
         gcore.Set(gcore.String(<char*?>args[0]), gcore.String(<char*?>args[1]), <bint?>args[2])
   
   @classmethod
   def ListPath(klass, char* key):
      cdef gcore.List[gcore.Path] pl
      cdef gcore.List[gcore.Path].iterator it
      gcore.ListPaths(gcore.String(key), pl)
      rv = []
      it = pl.begin()
      while it != pl.end():
         rv.append(deref(it).fullname('/').c_str())
         pinc(it)
      return rv
   

# cdef SetEnvPtr(Env py, gcore.Env* c, own):
#    if py._cobj != NULL:
#       if py._cobj == c:
#          return
#       if py._own:
#          del py._cobj
#    py._cobj = c
#    py._own = own
