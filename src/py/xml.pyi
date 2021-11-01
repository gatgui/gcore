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

ctypedef public class XMLElement [object PyXMLElement, type PyXMLElementType]:
   cdef gcore.XMLElement *_cobj
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
         self._cobj = new gcore.XMLElement()
      elif len(args) == 1:
         self._cobj = new gcore.XMLElement(gcore.String(<char*?>args[0]))
      else:
         raise Exception("_gcore.XMLElement() accepts at most 1 argument")

      self._own = True

   def __dealloc__(self):
      if self._own and self._cobj != NULL:
         del(self._cobj)
         self._cobj = NULL

   def addChild(self, c):
      # Steal ref from c
      if self._cobj.addChild((<XMLElement?>c)._cobj):
         (<XMLElement>c)._own = False
         return True
      else:
         return False

   def removeChild(self, c):
      if isinstance(c, XMLElement):
         self._cobj.removeChild((<XMLElement>c)._cobj)
      else:
         self._cobj.removeChild(<size_t?>c)

   def parent(self):
      cdef gcore.XMLElement *pe = self._cobj.parent()
      if pe != NULL:
         rv = XMLElement(noalloc=True)
         SetXMLElementPtr(rv, pe, False)
         return rv
      else:
         return None

   def childCount(self):
      return self._cobj.childCount()

   def child(self, idx):
      cdef gcore.XMLElement *pe = self._cobj.child(<size_t?>idx)
      if pe != NULL:
         rv = XMLElement(noalloc=True)
         SetXMLElementPtr(rv, pe, False)
         return rv
      else:
         return None

   def setAttribute(self, name, val):
      return self._cobj.setAttribute(gcore.String(<char*?>name), gcore.String(<char*?>val))

   def removeAttribute(self, name):
      self._cobj.removeAttribute(gcore.String(<char*?>name))

   def hasAttribute(self, name):
      return self._cobj.hasAttribute(gcore.String(<char*?>name))

   def attribute(self, name):
      return self._cobj.attribute(gcore.String(<char*?>name)).c_str()
   
   def getAttributes(self):
      cdef map[gcore.String,gcore.String] cattrs
      cdef map[gcore.String,gcore.String].iterator it

      self._cobj.getAttributes(cattrs)
      pattrs = {}

      it = cattrs.begin()
      while it != cattrs.end():
         pattrs[deref(it).first.c_str()] = deref(it).second.c_str()
         pinc(it)

      return pattrs

   def setText(self, v, asCDATA=False):
      return self._cobj.setText(gcore.String(<char*?>v), <bint?>asCDATA)

   def addText(self, v):
      return self._cobj.addText(gcore.String(<char*?>v))

   def text(self):
      return self._cobj.text().c_str()

   property tag:
      def __get__(self): return self._cobj.tag().c_str()
      def __set__(self, v): self._cobj.setTag(gcore.String(<char*?>v))
   
   def hasChildWithTag(self, t):
      return self._cobj.hasChildWithTag(gcore.String(<char*?>t))

   def countChildrenWithTag(self, t):
      return self._cobj.countChildrenWithTag(gcore.String(<char*?>t))
   
   def childWithTag(self, t, i=0):
      cdef gcore.XMLElement *pe = self._cobj.childWithTag(gcore.String(<char*?>t), <size_t?>i)
      if pe != NULL:
         rv = XMLElement(noalloc=True)
         SetXMLElementPtr(rv, pe, False)
         return rv
      else:
         return None

   def getChildrenWithTag(self, t):
      cdef gcore.List[gcore.XMLElement*] l
      cdef size_t n = self._cobj.getChildrenWithTag(gcore.String(<char*?>t), l)
      cdef size_t i = 0
      rv = []
      while i < n:
         e = XMLElement(noalloc=True)
         SetXMLElementPtr(e, l[i], False)
         rv.append(e)
         i += 1
      return rv


cdef SetXMLElementPtr(XMLElement py, gcore.XMLElement* c, own):
   if py._cobj != NULL:
      if py._cobj == c:
         return
      if py._own:
         del py._cobj
   py._cobj = c
   py._own = own


ctypedef public class XMLDoc [object PyXMLDoc, type PyXMLDocType]:
   cdef gcore.XMLDoc *_cobj
   cdef bint _own

   def __cinit__(self, *args, **kwargs):
      self._cobj = NULL
      self._own = False

   def __init__(self, *args, noalloc=False, **kwargs):
      if noalloc:
         self._cobj = NULL
         self._own = False

      self._cobj = new gcore.XMLDoc()
      self._own = True

   def __dealloc__(self):
      if self._own and self._cobj != NULL:
         del(self._cobj)
         self._cobj = NULL

   def rootCount(self):
      return self._cobj.rootCount()
   
   def setRoot(self, e):
      if e:
         (<XMLElement?>e)._own = False
      self._cobj.setRoot((<XMLElement?>e)._cobj)

   def addRoot(self, e):
      if e:
         (<XMLElement?>e)._own = False
      self._cobj.addRoot((<XMLElement?>e)._cobj)

   def root(self, idx=0):
      cdef gcore.XMLElement *r = self._cobj.root(<size_t?>idx)
      if r != NULL:
         rv = XMLElement(noalloc=True)
         SetXMLElementPtr(rv, r, False)
         return rv
      else:
         return None

   def write(self, path):
      self._cobj.write(gcore.String(<char*?>path))

   def read(self, path):
      return self._cobj.read(gcore.String(<char*?>path))

