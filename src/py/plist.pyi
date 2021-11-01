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


ctypedef public class PropertyList [object PyPropertyList, type PyPropertyListType]:
   cdef gcore.PropertyList *_cobj
   cdef bint _own

   def __cinit__(self, *args, **kwargs):
      self._cobj = NULL
      self._own = False

   def __init__(self, *args, noalloc=False, **kwargs):
      if noalloc:
         self._cobj = NULL
         self._own = False
         return

      self._cobj = new gcore.PropertyList()
      self._own = True

   def __dealloc__(self):
      if self._own and self._cobj != NULL:
         del(self._cobj)
         self._cobj = NULL

   def create(self):
      self._cobj.create()

   def read(self, arg):
      cdef gcore.Status stat
      if isinstance(arg, XMLElement):
         stat = self._cobj.read((<XMLElement>arg)._cobj)
      else:
         stat = self._cobj.read(gcore.String(<char*?>arg))
      return stat.succeeded()

   def write(self, arg):
      cdef gcore.XMLElement *rv = NULL

      if not arg:
         rv = self._cobj.write(NULL)
      elif isinstance(arg, XMLElement):
         rv = self._cobj.write((<XMLElement>arg)._cobj)
      else:
         self._cobj.write(gcore.String(<char*?>arg))
         rv = NULL
      if not rv:
         return None
      else:
         ret = XMLElement()
         SetXMLElementPtr(ret, rv, True)
         return ret

   def has(self, name):
      return self._cobj.has(gcore.String(<char*?>name))

   def remove(self, name):
      return self._cobj.remove(gcore.String(<char*?>name))

   def clear(self, name):
      cdef gcore.Status stat
      stat = self._cobj.clear(gcore.String(<char*?>name))
      if not stat.succeeded():
         raise Exception(stat.message())

   def size(self, name):
      cdef gcore.Status stat
      rv = self._cobj.size(gcore.String(<char*?>name), &stat)
      if not stat.succeeded():
         raise Exception(stat.message())
      return rv

   def getKeys(self, name):
      cdef gcore.Status stat
      cdef gcore.StringList l
      cdef size_t n = self._cobj.getKeys(gcore.String(<char*?>name), l, &stat)
      cdef size_t i = 0
      if not stat.succeeded():
         raise Exception(stat.message())
      rv = []
      while i < n:
         rv.append(l[i].c_str())
         i += 1
      return rv

   def asString(self, name, defaultValue=None):
      cdef gcore.Status stat
      cdef gcore.String rv
      if defaultValue is not None:
         rv = self._cobj.asString(gcore.String(<char*?>name), gcore.String(<char*?>defaultValue))
      else:
         rv = self._cobj.asString(gcore.String(<char*?>name), &stat)
         if not stat.succeeded():
            raise Exception(stat.message())
      return rv.c_str()

   def asInteger(self, name, defaultValue=None):
      cdef gcore.Status stat
      if defaultValue is not None:
         return self._cobj.asInteger(gcore.String(<char*?>name), <long?>defaultValue)
      else:
         rv = self._cobj.asInteger(gcore.String(<char*?>name), &stat)
         if not stat.succeeded():
            raise Exception(stat.message())
         return rv

   def asReal(self, name, defaultValue=None):
      cdef gcore.Status stat
      if defaultValue is not None:
         return self._cobj.asReal(gcore.String(<char*?>name), <double?>defaultValue)
      else:
         rv = self._cobj.asReal(gcore.String(<char*?>name), &stat)
         if not stat.succeeded():
            raise Exception(stat.message())
         return rv

   def asBoolean(self, name, defaultValue=None):
      cdef gcore.Status stat
      if defaultValue is not None:
         return self._cobj.asBoolean(gcore.String(<char*?>name), <bint?>defaultValue)
      else:
         rv = self._cobj.asBoolean(gcore.String(<char*?>name), &stat)
         if not stat.succeeded():
            raise Exception(stat.message())
         return rv

   def getString(self, name):
      cdef gcore.String val
      cdef gcore.Status st
      st = self._cobj.getString(gcore.String(<char*?>name), val)
      return (st.succeeded(), val.c_str())

   def getInteger(self, name):
      cdef long val = 0
      cdef gcore.Status st
      st = self._cobj.getInteger(gcore.String(<char*?>name), val)
      return (st.succeeded(), val)

   def getReal(self, name):
      cdef double val = 0.0
      cdef gcore.Status st
      st = self._cobj.getReal(gcore.String(<char*?>name), val)
      return (st.succeeded(), val)

   def getBoolean(self, name):
      cdef bint val = False
      cdef gcore.Status st
      # This confuses cython...
      #st = self._cobj.getBoolean(gcore.String(<char*?>name), <bint>val)
      val = self._cobj.asBoolean(gcore.String(<char*?>name), &st)
      return (st.succeeded(), val)

   def setString(self, name, v):
      cdef gcore.Status stat
      stat = self._cobj.setString(gcore.String(<char*?>name), gcore.String(<char*?>v))
      if not stat.succeeded():
         raise Exception(stat.message())

   def setInteger(self, name, v):
      cdef gcore.Status stat
      stat = self._cobj.setInteger(gcore.String(<char*?>name), <int>v)
      if not stat.succeeded():
         raise Exception(stat.message())

   def setReal(self, name, v):
      cdef gcore.Status stat
      stat = self._cobj.setReal(gcore.String(<char*?>name), <double>v)
      if not stat.succeeded():
         raise Exception(stat.message())

   def setBoolean(self, name, v):
      cdef gcore.Status stat
      stat = self._cobj.setBoolean(gcore.String(<char*?>name), <bint>v)
      if not stat.succeeded():
         raise Exception(stat.message())


