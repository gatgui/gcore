cimport gcore
from cython.operator cimport dereference as deref
#from cython.operator cimport reference as ref

ctypedef public class PerfLog [object PyPerfLog, type PyPerfLogType]:
   cdef gcore.PerfLog *_cobj
   cdef bint _own
   
   @classmethod
   def SharedInstance(klass):
      rv = PerfLog(noalloc=True)
      SetPerfLogPtr(rv, &(gcore.SharedInstance()), False)
      return rv
   
   @classmethod
   def Begin(klass, s):
      gcore.Begin(gcore.string(<char*?>s))
   
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
      
      # TODO
   
   def __dealloc__(self):
      if self._own and self._cobj != NULL:
         del(self._cobj)
         self._cobj = NULL
   
   def begin(self, s):
      self._cobj.begin(gcore.string(<char*?>s))
   
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
         
   
   
