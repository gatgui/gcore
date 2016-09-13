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

ctypedef public class Process [object PyProcess, type PyProcessType]:
   cdef gcore.Process *_cobj
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
         self._cobj = new gcore.Process()
      else:
         self._cobj = new gcore.Process()
         if len(args) == 1:
            self.run(args[0], **kwargs)
         else:
            self.run(args[0], **kwargs)
      
      self._own = True
   
   def __dealloc__(self):
      if self._own and self._cobj != NULL:
         del(self._cobj)
         self._cobj = NULL
   
   def setEnv(self, key, value):
      self._cobj.setEnv(gcore.String(<char*?>key), gcore.String(<char*?>value))
   
   def run(self, cmdOrArgs, **kwargs):
      cdef gcore.Status st
      cdef gcore.StringList args
      
      for k, v in kwargs.iteritems():
         if hasattr(self, k):
            setattr(self, k, v)
      
      t = type(cmdOrArgs)
      if t in (str, unicode):
         st = self._cobj.run(gcore.String(<char*?>cmdOrArgs))
      elif t in (tuple, list):
         for item in cmdOrArgs:
            args.push_back(gcore.String(<char*?>item))
         st = self._cobj.run(args)
      else:
         raise Exception("_gcore.Process.run: Invalid arguments (string or list expected)")
      
      if st.failed():
         raise Exception(st.message())
      
      return self._cobj.id()
   
   def canReadOut(self):
      return self._cobj.canReadOut()
   
   def canReadErr(self):
      return self._cobj.canReadErr()
   
   def canWriteIn(self):
      return self._cobj.canWriteIn()
   
   def read(self, block=True):
      cdef char tmp[256]
      cdef gcore.Status st
      
      readOut = (self._cobj.redirectOut() or (self._cobj.redirectErr() and self._cobj.redirectErrToOut()))
      readErr = (self._cobj.redirectErr() and not self._cobj.redirectErrToOut())
      
      out = ""
      err = ""
      
      while readOut or readErr:
         if readOut:
            readLen = self._cobj.readOut(tmp, 256, &st)
            if st.failed(): # readLen is 0
               if not self._cobj.canReadOut():
                  #raise Exception(st.message())
                  print("_gcore.Process.read: Error occured while reading 'out' pipe (%s)" % st.message())
                  readOut = False
               else:
                  readOut = block
            else:
               if readLen > 0:
                  out += tmp
                  readOut = block
               else:
                  # EOF
                  readOut = False
         
         if readErr:
            readLen = self._cobj.readErr(tmp, 256, &st)
            if st.failed(): # readLen is 0
               if not self._cobj.canReadErr():
                  #raise Exception(st.message())
                  print("_gcore.Process.read: Error occured while reading 'err' pipe (%s)" % st.message())
                  readErr = False
               else:
                  readErr = block
            else:
               if readLen > 0:
                  err += tmp
                  readErr = block
               else:
                  # EOF
                  readErr = False
      
      return out, err
   
   def write(self, msg):
      cdef gcore.Status st
      rv = self._cobj.write(gcore.String(<char*?>msg), &st)
      if st.failed():
         #raise Exception(st.message())
         print("_gcore.Process.write: Error occured while writing 'in' pipe (%s)" % st.message())
      return rv
   
   def isRunning(self):
      return self._cobj.isRunning()
   
   def wait(self, blocking):
      cdef gcore.Status st
      rv = self._cobj.wait(blocking, &st)
      if st.failed():
         #raise Exception(st.message())
         print("gcore.Process.wait: %s" % st.message())
      return rv
   
   def kill(self):
      cdef gcore.Status st
      st = self._cobj.kill()
      return st.succeeded()
   
   def returnCode(self):
      return self._cobj.returnCode()
   
   def cmdLine(self):
      return self._cobj.cmdLine().c_str()
   
   property redirectOut:
      def __get__(self): return self._cobj.redirectOut()
      def __set__(self, v): self._cobj.setRedirectOut(<bint?>v)
   
   property redirectErr:
      def __get__(self): return self._cobj.redirectErr()
      def __set__(self, v): self._cobj.setRedirectErr(<bint?>v)
   
   property redirectErrToOut:
      def __get__(self): return self._cobj.redirectErrToOut()
      def __set__(self, v): self._cobj.setRedirectErrToOut(<bint?>v)
   
   property redirectIn:
      def __get__(self): return self._cobj.redirectIn()
      def __set__(self, v): self._cobj.setRedirectIn(<bint?>v)
   
   property showConsole:
      def __get__(self): return self._cobj.showConsole()
      def __set__(self, v): self._cobj.setShowConsole(<bint?>v)
   
   property keepAlive:
      def __get__(self): return self._cobj.keepAlive()
      def __set__(self, v): self._cobj.setKeepAlive(<bint?>v)
   