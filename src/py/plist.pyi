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
      if isinstance(arg, XMLElement):
         return self._cobj.read((<XMLElement>arg)._cobj)
      else:
         return self._cobj.read(gcore.String(<char*?>arg))

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
      self._cobj.clear(gcore.String(<char*?>name))

   def getSize(self, name):
      return self._cobj.getSize(gcore.String(<char*?>name))

   def getKeys(self, name):
      cdef gcore.StringList l
      cdef size_t n = self._cobj.getKeys(gcore.String(<char*?>name), l)
      cdef size_t i = 0
      rv = []
      while i < n:
         rv.append(l[i].c_str())
         i += 1
      return rv

   def getString(self, name):
      return self._cobj.getString(gcore.String(<char*?>name)).c_str()

   def getInteger(self, name):
      return self._cobj.getInteger(gcore.String(<char*?>name))

   def getReal(self, name):
      return self._cobj.getReal(gcore.String(<char*?>name))

   def getBoolean(self, name):
      return self._cobj.getBoolean(gcore.String(<char*?>name))

   def setString(self, name, v):
      self._cobj.setString(gcore.String(<char*?>name), gcore.String(<char*?>v))

   def setInteger(self, name, v):
      self._cobj.setInteger(gcore.String(<char*?>name), <int>v)

   def setReal(self, name, v):
      self._cobj.setReal(gcore.String(<char*?>name), <double>v)

   def setBoolean(self, name, v):
      self._cobj.setBoolean(gcore.String(<char*?>name), <bint>v)


