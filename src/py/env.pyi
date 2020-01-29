# Copyright (C) 2010~  Gaetan Guidet
# 
# This file is part of gcore.
# 
# gcore is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 2.1 of the License, or (at
# your option) any later version.
# 
# gcore is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
# USA.

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
   
   def isSet(self, key):
      cdef gcore.String _key
      _to_cstring(key, _key)
      return self._cobj.isSet(_key)
   
   def get(self, key):
      cdef gcore.String _key
      cdef gcore.String _val
      _to_cstring(key, _key)
      _val = self._cobj.get(_key)
      return _to_pystring(_val, asUnicode=False)
   
   def set(self, *args):
      cdef map[gcore.String, gcore.String, gcore.KeyCompare] cd
      cdef gcore.String _key
      cdef gcore.String _val
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
            _to_cstring(k, _key)
            _to_cstring(v, _val)
            cd[_key] = _val
         self._cobj.set(cd, <bint?>args[1])
      else:
         _to_cstring(args[0], _key)
         _to_cstring(args[1], _val)
         self._cobj.set(_key, _val, <bint?>args[2])
   
   def unset(self, key):
      cdef gcore.String _key
      _to_cstring(key, _key)
      self._cobj.unset(_key)

   def asDict(self):
      cdef gcore.String _key
      cdef gcore.String _val
      cdef map[gcore.String, gcore.String, gcore.KeyCompare] cd
      cdef map[gcore.String, gcore.String, gcore.KeyCompare].iterator it
      self._cobj.asDict(cd)
      rv = EnvDict()
      it = cd.begin()
      while it != cd.end():
         _key = deref(it).first
         _val = deref(it).second
         rv[_to_pystring(_key, asUnicode=False)] = _to_pystring(_val, asUnicode=False)
         pinc(it)
      return rv
   
   @classmethod
   def Username(klass):
      return gcore.Username().c_str()
   
   @classmethod
   def Hostname(klass):
      return gcore.Hostname().c_str()
   
   @classmethod
   def IsSet(klass, key):
      cdef gcore.String _key
      _to_cstring(key, _key)
      return gcore.IsSet(_key)
   
   @classmethod
   def Get(klass, key):
      cdef gcore.String _key
      cdef gcore.String _val
      _to_cstring(key, _key)
      _val = gcore.Get(_key)
      return _to_pystring(_val, asUnicode=False)
   
   @classmethod
   def Set(klass, *args):
      cdef gcore.String _key
      cdef gcore.String _val
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
            _to_cstring(k, _key)
            _to_cstring(v, _val)
            cd[_key] = _val
         gcore.Set(cd, <bint?>args[1])
      else:
         _to_cstring(args[0], _key)
         _to_cstring(args[1], _val)
         gcore.Set(_key, _val, <bint?>args[2])
   
   @classmethod
   def Unset(klass, key):
      cdef gcore.String _key
      _to_cstring(key, _key)
      gcore.Unset(_key)
   
   @classmethod
   def ListPath(klass, key):
      cdef gcore.String _key
      cdef gcore.String _val
      cdef gcore.List[gcore.Path] pl
      cdef gcore.List[gcore.Path].iterator it
      _to_cstring(key, _key)
      gcore.ListPaths(_key, pl)
      rv = []
      it = pl.begin()
      while it != pl.end():
         _val = deref(it).fullname('/')
         rv.append(_to_pystring(_val, asUnicode=True))
         pinc(it)
      return rv
   
