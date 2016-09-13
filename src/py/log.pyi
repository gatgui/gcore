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
from cython.operator import dereference as deref
import sys

LOG_ERROR = gcore.LOG_ERROR
LOG_WARNING = gcore.LOG_WARNING
LOG_DEBUG = gcore.LOG_DEBUG
LOG_INFO = gcore.LOG_INFO
LOG_ALL = gcore.LOG_ALL

ctypedef public class Log [object PyLog, type PyLogType]:
   cdef gcore.Log *_cobj
   cdef gcore.LogOutputFunc *_outfunc
   cdef bint _own
   
   @classmethod
   def SetLevelMask(klass, int mask):
      gcore.SetLevelMask(mask)
   
   @classmethod
   def LevelMask(klass):
      return gcore.LevelMask()
   
   @classmethod
   def SetIndentLevel(klass, int l):
      gcore.SetIndentLevel(l)
   
   @classmethod
   def IndentLevel(klass):
      return gcore.IndentLevel()
   
   @classmethod
   def Indent(klass):
      gcore.Indent()
   
   @classmethod
   def UnIndent(klass):
      gcore.UnIndent()
   
   @classmethod
   def SetIndentWidth(klass, int w):
      gcore.SetIndentWidth(w)
   
   @classmethod
   def IndentWidth(klass):
      return gcore.IndentWidth()
   
   @classmethod
   def SetColorOutput(klass, bint onoff):
      gcore.SetColorOutput(onoff)
   
   @classmethod
   def ColorOutput(klass):
      return gcore.ColorOutput()
   
   @classmethod
   def SetShowTimeStamps(klass, bint onoff):
      gcore.SetShowTimeStamps(onoff)
   
   @classmethod
   def ShowTimeStamps(klass):
      return gcore.ShowTimeStamps()
   
   @classmethod
   def PrintError(klass, char* msg):
      gcore.PrintError(msg)
   
   @classmethod
   def PrintWarning(klass, char* msg):
      gcore.PrintWarning(msg)
   
   @classmethod
   def PrintDebug(klass, char* msg):
      gcore.PrintDebug(msg)
   
   @classmethod
   def PrintInfo(klass, char* msg):
      gcore.PrintInfo(msg)
   
   @classmethod
   def SetOutputFunc(klass, outputFunc):
      gcore.PyLog_SetOutputFunc(<gcore.PyObject*>outputFunc)
   
   
   def __cinit__(self, *args, **kwargs):
      self._cobj = NULL
      self._own = False
   
   def __init__(self, *args, noalloc=False, **kwargs):
      self._outfunc = new gcore.LogOutputFunc()

      if noalloc:
         self._cobj = NULL
         self._own = False
         return
      
      if len(args) == 0:
         self._cobj = new gcore.Log()
      elif len(args) == 1:
         if isinstance(args[0], Path):
            self._cobj = new gcore.Log(deref((<Path>args[0])._cobj))
         elif isinstance(args[0], Log):
            self._cobj = new gcore.Log(deref((<Log>args[0])._cobj))
         elif type(args[0]) in [str, unicode]:
            self._cobj = new gcore.Log(gcore.Path(<char*>args[0]))
         else:
            raise Exception("_gcore.Log() invalid argument type %s" % type(str))
      else:
         raise Exception("_gcore.Log() accepts at most 1 argument")
      
      self._own = True
   
   def __dealloc__(self):
      if self._own and self._cobj != NULL:
         del(self._cobj)
         self._cobj = NULL
      del(self._outfunc)
   
   def _printStdout(self, msg):
      sys.stdout.write(msg)
   
   def printError(self, msg):
      self._cobj.printError(<char*?>msg)
   
   def printWarning(self, msg):
      self._cobj.printWarning(<char*?>msg)
   
   def printDebug(self, msg):
      self._cobj.printDebug(<char*?>msg)
   
   def printInfo(self, msg):
      self._cobj.printInfo(<char*?>msg)
   
   property levelMask:
      def __get__(self): return self._cobj.levelMask()
      def __set__(self, v): self._cobj.setLevelMask(<unsigned int>v)
   
   property indentLevel:
      def __get__(self): return self._cobj.indentLevel()
      def __set__(self, v): self._cobj.setIndentLevel(<unsigned int>v)
   
   property indentWidth:
      def __get__(self): return self._cobj.indentWidth()
      def __set__(self, v): self._cobj.setIndentWidth(<unsigned int>v)
   
   def indent(self):
      self._cobj.indent()
   
   def unIndent(self):
      self._cobj.unIndent()
   
   property colorOutput:
      def __get__(self): return self._cobj.colorOutput()
      def __set__(self, v): self._cobj.setColorOutput(<bint>v)
   
   property showTimeStamps:
      def __get__(self): return self._cobj.showTimeStamps()
      def __set__(self, v): self._cobj.setShowTimeStamps(<bint>v)
   
   def setOutputFunc(self, func):
      if func is None:
         self._outfunc.setPyFunc(<gcore.PyObject*>self._printStdout)
      else:
         self._outfunc.setPyFunc(<gcore.PyObject*>func)
      self._outfunc.assign(deref(self._cobj))
   
