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
#from cython.operator cimport reference as ref

ctypedef public class PerfLog [object PyPerfLog, type PyPerfLogType]:
   cdef gcore.PerfLog *_cobj
   cdef bint _own
   
   @classmethod
   def Get(klass):
      rv = PerfLog(noalloc=True)
      SetPerfLogPtr(rv, &(gcore.Get()), False)
      return rv
   
   @classmethod
   def Begin(klass, s):
      gcore.Begin(gcore.String(<char*?>s))
   
   @classmethod
   def End(klass):
      gcore.End()
   
   @classmethod
   def Print(klass, log=None, output=gcore.ConsoleOutput, flags=gcore.ShowDefaults, sortBy=gcore.SortFuncTime, units=gcore.CurrentUnits):
      if log is None:
         gcore.Print(<gcore.Output>output, <int>flags, <int>sortBy, <gcore.Units>units)
      else:
         gcore.Print(deref((<Log?>log)._cobj), <int>flags, <int>sortBy, <gcore.Units>units)
   
   @classmethod
   def Clear(klass):
      gcore.Clear()
   
   @classmethod
   def UnitsString(klass, units):
      return gcore.UnitsString(<gcore.Units>units)
   
   @classmethod
   def ConvertUnits(klass, v, src, dst):
      return gcore.ConvertUnits(<double?>v, <gcore.Units>src, <gcore.Units>dst)
   
   
   def __cinit__(self, *args, **kwargs):
      self._cobj = NULL
      self._own = False
   
   def __init__(self, *args, noalloc=False, **kwargs):
      if noalloc:
         self._cobj = NULL
         self._own = False
         return
      
      if len(args) == 0:
         self._cobj = new gcore.PerfLog()
      elif len(args) == 1:
         self._cobj = new gcore.PerfLog(<gcore.Units>args[0])
      else:
         raise Exception("_gcore.PerfLog() accepts at most 1 argument")
      
      self._own = True
   
   def __dealloc__(self):
      if self._own and self._cobj != NULL:
         del(self._cobj)
         self._cobj = NULL
   
   def begin(self, s):
      self._cobj.begin(gcore.String(<char*?>s))
   
   def end(self):
      self._cobj.end()
   
   def print_(self, log=None, output=gcore.ConsoleOutput, flags=gcore.ShowDefaults, sortBy=gcore.SortFuncTime, units=gcore.CurrentUnits):
      if log is None:
         self._cobj._print(<gcore.Output>output, <int>flags, <int>sortBy, <gcore.Units>units)
      else:
         self._cobj._print(deref((<Log?>log)._cobj), <int>flags, <int>sortBy, <gcore.Units>units)
   
   def clear(self):
      self._cobj.clear()
   

cdef SetPerfLogPtr(PerfLog py, gcore.PerfLog* c, own):
   if py._cobj != NULL:
      if py._cobj == c:
         return
      if py._own:
         del py._cobj
   py._cobj = c
   py._own = own

# A decorator for logperf
def logperf(msg=None, log=None):
   def wrap1(func):
      def wrap2(*args, **kwargs):
         if log is None:
            PerfLog.Begin(func.__name__ if msg is None else msg)
         else:
            log.begin(func.__name__ if msg is None else msg)
         rv = func(*args, **kwargs)
         if log is None:
            PerfLog.End()
         else:
            log.end()
         return rv
      
      return wrap2
   
   return wrap1
         
   
   
