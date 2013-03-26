cimport gcore
from cython.operator cimport dereference as deref

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
      return self._cobj.addChild((<XMLElement>c)._cobj)

   def removeChild(self, c):
      if isinstance(c, XMLElement):
         self._cobj.removeChild((<XMLElement>c)._cobj)
      else:
         self._cobj.removeChild(<size_t>c)

   def getParent(self):
      cdef gcore.XMLElement *pe = self._cobj.getParent()
      if pe != NULL:
         rv = XMLElement(noalloc=True)
         SetXMLElementPtr(rv, pe, False)
         return rv
      else:
         return None

   def numChildren(self):
      return self._cobj.numChildren()

   def getChild(self, idx):
      cdef gcore.XMLElement *pe = self._cobj.getChild(<size_t>idx)
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

   def getAttribute(self, name):
      return self._cobj.getAttribute(gcore.String(<char*?>name)).c_str()
   
   def setText(self, v, asCDATA=False):
      return self._cobj.setText(gcore.String(<char*?>v), <bint?>asCDATA)

   def addText(self, v):
      return self._cobj.addText(gcore.String(<char*?>v))

   def getText(self):
      return self._cobj.getText().c_str()

   def setTag(self, t):
      self._cobj.setTag(gcore.String(<char*?>t))

   def getTag(self):
      return self._cobj.getTag().c_str()
   
   def hasChildWithTag(self, t):
      return self._cobj.hasChildWithTag(gcore.String(<char*?>t))

   def numChildrenWithTag(self, t):
      return self._cobj.numChildrenWithTag(gcore.String(<char*?>t))
   
   def getChildWithTag(self, t, i):
      cdef gcore.XMLElement *pe = self._cobj.getChildWithTag(gcore.String(<char*?>t), <size_t>i)
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

   def numRoots(self):
      return self._cobj.numRoots()
   
   def setRoot(self, e):
      self._cobj.setRoot((<XMLElement>e)._cobj)

   def addRoot(self, e):
      self._cobj.addRoot((<XMLElement>e)._cobj)

   def getRoot(self, idx=0):
      cdef gcore.XMLElement *r = self._cobj.getRoot(<size_t?>idx)
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

