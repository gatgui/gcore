# MIT License
#
# Copyright (c) 2010 Gaetan Guidet
#
# This file is part of gcore.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


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
   
   def set(self, char* key, char* val, bint overwrite):
      self._cobj.set(gcore.String(key), gcore.String(val), overwrite)
   
   def setAll(self, edict, bint overwrite):
      cdef map[gcore.String, gcore.String] cd
      for k, v in edict.iteritems():
         cd[gcore.String(<char*?>k)] = gcore.String(<char*?>v)
      self._cobj.setAll(cd, overwrite)
   
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
   def GetUser(klass):
      return gcore.GetUser().c_str()
   
   @classmethod
   def GetHost(klass):
      return gcore.GetHost().c_str()
   
   @classmethod
   def IsSet(klass, char* key):
      return gcore.IsSet(gcore.String(key))
   
   @classmethod
   def Get(klass, char* key):
      return gcore.Get(gcore.String(key)).c_str()
   
   @classmethod
   def Set(klass, char* key, char* val, bint overwrite):
      gcore.Set(gcore.String(key), gcore.String(val), overwrite)
   
   @classmethod
   def SetAll(klass, edict, bint overwrite):
      cdef map[gcore.String, gcore.String] cd
      for k, v in edict.iteritems():
         cd[gcore.String(<char*?>k)] = gcore.String(<char*?>v)
      gcore.SetAll(cd, overwrite)
   
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
