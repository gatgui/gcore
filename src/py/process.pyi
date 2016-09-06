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
      elif len(args) == 1:
         self._cobj = new gcore.Process()
         self.run(args[0], **kwargs)
      else:
         raise Exception("_gcore.Process() doesn't accept any argument")
      
      self._own = True
   
   def __dealloc__(self):
      if self._own and self._cobj != NULL:
         del(self._cobj)
         self._cobj = NULL
   
   def setEnv(self, key, value):
      self._cobj.setEnv(gcore.String(<char*?>key), gcore.String(<char*?>value))
   
   def run(self, cmd, **kwargs):
      for k, v in kwargs.iteritems():
         if hasattr(self, k):
            setattr(self, k, v)
      pid = self._cobj.run(gcore.String(<char*?>cmd))
      if sys.platform == "win32":
         if pid == 0:
            raise Exception("[gcore.Process.run]")
      else:
         if pid == -1:
            raise Exception("[gcore.Process.run]")
      return pid
   
   def read(self, block=True):
      cdef gcore.String tmp
      
      readOut = (self._cobj.captureOut() or (self._cobj.captureErr() and self._cobj.redirectErrToOut()))
      readErr = (self._cobj.captureErr() and not self._cobj.redirectErrToOut())
      
      out = ""
      err = ""
      
      while readOut or readErr:
         if readOut:
            readLen = self._cobj.read(tmp)
            if readLen > 0:
               out += tmp.c_str()
               readOut = block
            elif readLen == 0:
               readOut = False
            else:
               readOut = False
               print("[gcore.Process.read: Error occured while reading 'out' pipe")
         
         if readErr:
            readLen = self._cobj.readErr(tmp)
            if readLen > 0:
               err += tmp.c_str()
               readErr = block
            elif readLen == 0:
               readErr = False
            else:
               readErr = False
               print("[gcore.Process.read: Error occured while reading 'err' pipe")
      
      return out, err
   
   def write(self, msg):
      return self._cobj.write(gcore.String(<char*?>msg))
   
   def running(self):
      return self._cobj.running()
   
   def wait(self, blocking):
      return self._cobj.wait(blocking)
   
   def kill(self):
      return self._cobj.kill()
   
   def returnCode(self):
      return self._cobj.returnCode()
   
   def getCmdLine(self):
      return self._cobj.getCmdLine().c_str()
   
   property captureOut:
      def __get__(self): return self._cobj.captureOut()
      def __set__(self, v): self._cobj.captureOut(v)
   
   property captureErr:
      def __get__(self): return self._cobj.captureErr()
      def __set__(self, v): self._cobj.captureErr(v, self._cobj.redirectErrToOut())
   
   property redirectErrToOut:
      def __get__(self): return self._cobj.redirectErrToOut()
      def __set__(self, v): self._cobj.captureErr(self._cobj.captureErr(), v)
   
   property redirectIn:
      def __get__(self): return self._cobj.redirectIn()
      def __set__(self, v): self._cobj.redirectIn(v)
   
   property showConsole:
      def __get__(self): return self._cobj.showConsole()
      def __set__(self, v): self._cobj.showConsole(v)
   
   property keepAlive:
      def __get__(self): return self._cobj.keepAlive()
      def __set__(self, v): self._cobj.keepAlive(v)
   
   property verbose:
      def __get__(self): return self._cobj.verbose()
      def __set__(self, v): self._cobj.verbose(v)
   