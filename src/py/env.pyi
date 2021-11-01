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


# Note: 'super' not used on purpose
class EnvDict(dict):
   class Key(str):
      def __init__(self, key):
         str.__init__(self, key)
         self._lstr = key.lower()

      def __hash__(self):
         return hash(self._lstr)

      def __rhs(self, other):
         return (other._lstr if isinstance(other, type(self)) else other.lower())

      def __eq__(self, other):
         return self._lstr == self.__rhs(other)

      def __ne__(self, other):
         return self._lstr == self.__rhs(other)

      def __gt__(self, other):
         return self._lstr > self.__rhs(other)

      def __lt__(self, other):
         return self._lstr < self.__rhs(other)

      def __ge__(self, other):
         return self._lstr >= self.__rhs(other)

      def __le__(self, other):
         return self._lstr <= self.__rhs(other)

   def __init__(self, *args, **kwargs):
      dict.__init__(self)
      if len(args) > 0:
         if len(args) == 1:
            if isinstance(args[0], dict):
               for key, val in args[0].iteritems():
                  self[key] = val
            else:
               for key, val in args[0]:
                  self[key] = val
         else:
            raise Exception("Expect a mapping or in iterable as argument")
      for key, val in kwargs.iteritems():
         self[key] = val

   def __contains__(self, key):
      return dict.__contains__(self, self.Key(key))

   def __setitem__(self, key, value):
      dict.__setitem__(self, self.Key(key), value)

   def __getitem__(self, key):
      return dict.__getitem__(self, self.Key(key))


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
      cdef map[gcore.String, gcore.String, gcore.KeyCompare] cd
      if len(args) < 2 or len(args) > 3:
         raise Exception("_gcore.Env.set takes 2 to 3 arguments")
      elif len(args) == 2:
         if not isinstance(args[0], dict):
            raise Exception("_gcore.Env.set expects a dict")
         if not isinstance(args[0], EnvDict):
            ed = EnvDict(args[0].items())
            if len(ed) != len(args[0]):
               dk = set(args[0].keys()).difference(ed.keys())
               raise Exception("_gcore.Env.set dict argument has duplicate environment keys for %s" % ", ".join(dk))
         for k, v in args[0].iteritems():
            cd[gcore.String(<char*?>k)] = gcore.String(<char*?>v)
         self._cobj.set(cd, <bint?>args[1])
      else:
         self._cobj.set(gcore.String(<char*?>args[0]), gcore.String(<char*?>args[1]), <bint?>args[2])
   
   def unset(self, char* key):
      self._cobj.unset(gcore.String(key))

   def asDict(self):
      cdef map[gcore.String, gcore.String, gcore.KeyCompare] cd
      cdef map[gcore.String, gcore.String, gcore.KeyCompare].iterator it
      self._cobj.asDict(cd)
      rv = EnvDict()
      it = cd.begin()
      while it != cd.end():
         rv[deref(it).first.c_str()] = deref(it).second.c_str()
         pinc(it)
      return rv
   
   @classmethod
   def Username(klass):
      return gcore.Username().c_str()
   
   @classmethod
   def Hostname(klass):
      return gcore.Hostname().c_str()
   
   @classmethod
   def IsSet(klass, char* key):
      return gcore.IsSet(gcore.String(key))
   
   @classmethod
   def Get(klass, char* key):
      return gcore.Get(gcore.String(key)).c_str()
   
   @classmethod
   def Set(klass, *args):
      cdef map[gcore.String, gcore.String, gcore.KeyCompare] cd
      if len(args) < 2 or len(args) > 3:
         raise Exception("_gcore.Env.Set takes 2 to 3 arguments")
      elif len(args) == 2:
         if not isinstance(args[0], dict):
            raise Exception("_gcore.Env.Set expects a dict")
         if not isinstance(args[0], EnvDict):
            ed = EnvDict(args[0].items())
            if len(ed) != len(args[0]):
               dk = set(args[0].keys()).difference(ed.keys())
               raise Exception("_gcore.Env.Set dict argument has duplicate environment keys for %s" % ", ".join(dk))
         for k, v in args[0].iteritems():
            cd[gcore.String(<char*?>k)] = gcore.String(<char*?>v)
         gcore.Set(cd, <bint?>args[1])
      else:
         gcore.Set(gcore.String(<char*?>args[0]), gcore.String(<char*?>args[1]), <bint?>args[2])
   
   @classmethod
   def Unset(klass, char* key):
      gcore.Unset(gcore.String(key))
   
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
