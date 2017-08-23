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
from cython.operator cimport dereference as deref
import sys

DIR_SEP = ("/" if sys.platform != "win32" else "\\")

ctypedef public class Path [object PyPath, type PyPathType]:
   cdef gcore.Path *_cobj
   cdef bint _own
   
   FE_FILE = gcore.FE_FILE
   FE_DIRECTORY = gcore.FE_DIRECTORY
   FE_HIDDEN = gcore.FE_HIDDEN
   FE_ALL = gcore.FE_ALL
   
   @classmethod
   def CurrentDir(klass):
      p = Path()
      p._cobj.assign(gcore.CurrentDir())
      return p
   
   def __cinit__(self, *args, **kwargs):
      self._cobj = NULL
      self._own = False
   
   def __init__(self, *args, noalloc=False, **kwargs):
      if noalloc:
         self._cobj = NULL
         self._own = False
         return
      
      if len(args) == 0:
         self._cobj = new gcore.Path()
      elif len(args) == 1:
         if type(args[0]) in [str, unicode]:
            self._cobj = new gcore.Path(<char*>args[0])
         else:
            self._cobj = new gcore.Path(deref((<Path?>args[0])._cobj))
      else:
         raise Exception("_gcore.Path() accepts at most 1 argument")
      
      self._own = True
      
   def __dealloc__(self):
      if self._own and self._cobj != NULL:
         del(self._cobj)
         self._cobj = NULL
   
   def push(self, s):
      self._cobj.push(gcore.String(<char*?>s))
      return self
   
   def pop(self):
      return self._cobj.pop().c_str()
   
   def depth(self):
      return self._cobj.depth()
   
   def remove(self, recursive):
      cdef gcore.Status stat
      stat = self._cobj.remove(<bint?>recursive)
      return stat.succeeded()
   
   def copy(self, to, recursive, createMissingDirs, overwrite):
      cdef gcore.Status stat
      stat = self._cobj.copy(deref((<Path?>to)._cobj), <bint?>recursive, <bint?>createMissingDirs, <bint?>overwrite)
      return stat.succeeded()
   
   def createDir(self, recursive):
      cdef gcore.Status stat
      stat = self._cobj.createDir(<bint?>recursive)
      return stat.succeeded()
   
   def isDir(self):
      return self._cobj.isDir()
   
   def isFile(self):
      return self._cobj.isFile()
   
   def exists(self):
      return self._cobj.exists()
   
   def fileSize(self):
      return self._cobj.fileSize()
   
   def isAbsolute(self):
      return self._cobj.isAbsolute()
   
   def makeAbsolute(self):
      self._cobj.makeAbsolute()
      return self
   
   def normalize(self):
      self._cobj.normalize()
      return self
   
   def forEach(self, func, recursive=False, flags=Path.FE_ALL):
      cdef gcore.PathEnumerator enumerator
      enumerator.setPyFunc(<gcore.PyObject*>func)
      enumerator.apply(deref(self._cobj), <bint?>recursive, <int>flags)
   
   def listDir(self, recursive=False, flags=Path.FE_ALL):
      cdef gcore.List[gcore.Path] paths
      cdef size_t i = 0
      cdef size_t n = self._cobj.listDir(paths, <bint?>recursive, <int?>flags)
      
      rv = []
      while i < n:
         p = Path()
         p._cobj.assign(paths[i])
         rv.append(p)
         i += 1
      
      return rv
   
   def __str__(self):
      return self._cobj.to_string().c_str()
   
   def __cmp__(self, rhs):
      if deref(self._cobj) == deref((<Path?>rhs)._cobj):
         return 0
      else:
         return 1 # or -1 who cares
   
   def __getitem__(self, idx):
      return deref(self._cobj)[<int?>idx].c_str()
   
   def __iadd__(self, rhs):
      if type(rhs) in [str, unicode]:
         self._cobj.plus_eq(gcore.Path(<char*>rhs))
      else:
         self._cobj.plus_eq(deref((<Path?>rhs)._cobj))
      return self
   
   def __add__(self, rhs):
      rv = Path(self)
      rv += rhs
      return rv
   
   def basename(self):
      return self._cobj.basename().c_str()
   
   def dirname(self, sep=None):
      if sep is None:
         return self._cobj.dirname('/').c_str()
      elif not type(sep) in [str, unicode] or len(sep) != 1:
         raise Exception("_gcore.Path.dirname expects a string argument of length 1")
      return self._cobj.dirname((<char*?>sep)[0]).c_str()
   
   def fullname(self, sep=None):
      if sep is None:
         return self._cobj.fullname('/').c_str()
      elif not type(sep) in [str, unicode] or len(sep) != 1:
         raise Exception("_gcore.Path.fullname expects a string argument of length 1")
      return self._cobj.fullname((<char*?>sep)[0]).c_str()
   
   def extension(self):
      return self._cobj.extension().c_str()
   
   def checkExtension(self, e):
      return self._cobj.checkExtension(gcore.String(<char*?>e))
   
   def setExtension(self, e):
      return self._cobj.setExtension(gcore.String(<char*?>e))
   
   def lastModification(self):
      import datetime, os
      st = os.stat(self.fullname())
      return datetime.datetime.fromtimestamp(st.st_mtime)
   

