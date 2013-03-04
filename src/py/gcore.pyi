cimport gcore
from libcpp.map cimport map
from cython.operator cimport dereference as deref
from cython.operator cimport preincrement as pinc

LOG_ERROR = gcore.LOG_ERROR
LOG_WARNING = gcore.LOG_WARNING
LOG_DEBUG = gcore.LOG_DEBUG
LOG_INFO = gcore.LOG_INFO
LOG_ALL = gcore.LOG_ALL

class Log(object):
   def __init__(self):
      object.__init__(self)
   
   @classmethod
   def SelectOutputs(klass, int flags):
      gcore.SelectOutputs(flags)
   
   @classmethod
   def SelectedOutputs(klass):
      return gcore.SelectedOutputs()
   
   @classmethod
   def SetIndentLevel(klass, int l):
      gcore.SetIndentLevel(l)
   
   @classmethod
   def GetIndentLevel(klass):
      return gcore.GetIndentLevel()
   
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
   def GetIndentWidth(klass):
      return gcore.GetIndentWidth()
   
   @classmethod
   def EnableColors(klass, bint onoff):
      gcore.EnableColors(onoff)
   
   @classmethod
   def ColorsEnabled(klass):
      return gcore.ColorsEnabled()
   
   @classmethod
   def ShowTimeStamps(klass, bint onoff):
      gcore.ShowTimeStamps(onoff)
   
   @classmethod
   def TimeStampsShown(klass):
      return gcore.TimeStampsShown()
   
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
   


ctypedef public class Env [object PyEnv, type PyEnvType]:
   cdef gcore.Env *_cobj
   cdef bint _own
   
   def __cinit__(self, *args, **kwargs):
      self._cobj = NULL
      self._own = False
   
   def __init__(self, noalloc=False):
      if noalloc:
         self._cobj = NULL
         self._own = False
         return
      self._cobj = new gcore.Env()
      self._own = True
   
   def __dealloc__(self):
      if self._own and self._cobj != NULL:
         del self._cobj
         self._cobj = NULL
   
   def push(self):
      self._cobj.push()
   
   def pop(self):
      self._cobj.pop()
   
   def isSet(self, char* key):
      return self._cobj.isSet(gcore.String(key))
   
   def get(self, char* key):
      return self._cobj.get(gcore.String(key)).c_str()
   
   def set(self, char* key, char* val, bint overwrite):
      self._cobj.set(gcore.String(key), gcore.String(val), overwrite)
   
   def setAll(self, edict, bint overwrite):
      cdef map[gcore.String, gcore.String] cd
      for k, v in edict.iteritems():
         cd[gcore.String(<char*?>k)] = gcore.String(<char*?>v)
      self._cobj.setAll(cd, overwrite)
   
   def asDict(self):
      cdef map[gcore.String, gcore.String] cd
      cdef map[gcore.String, gcore.String].iterator it
      self._cobj.asDict(cd)
      rv = {}
      it = cd.begin()
      while it != cd.end():
         rv[deref(it).first.c_str()] = deref(it).second.c_str()
         pinc(it)
      return rv
   
   @classmethod
   def GetUser(klass):
      return gcore.GetUser().c_str()
   
   @classmethod
   def GetHost(klass):
      return gcore.GetHost().c_str()
   
   @classmethod
   def IsSet(klass, char* key):
      return gcore.IsSet(gcore.String(key))
   
   @classmethod
   def Get(klass, char* key):
      return gcore.Get(gcore.String(key)).c_str()
   
   @classmethod
   def Set(klass, char* key, char* val, bint overwrite):
      gcore.Set(gcore.String(key), gcore.String(val), overwrite)
   
   @classmethod
   def SetAll(klass, edict, bint overwrite):
      cdef map[gcore.String, gcore.String] cd
      for k, v in edict.iteritems():
         cd[gcore.String(<char*?>k)] = gcore.String(<char*?>v)
      gcore.SetAll(cd, overwrite)
   
   @classmethod
   def ListPath(klass, char* key):
      cdef gcore.List[gcore.Path] pl
      cdef gcore.List[gcore.Path].iterator it
      gcore.ListPaths(gcore.String(key), pl)
      rv = []
      it = pl.begin()
      while it != pl.end():
         rv.append(deref(it).fullname('/').c_str())
         pinc(it)
      return rv
   


cdef SetEnvPtr(Env py, gcore.Env* c, own):
   if py._cobj != NULL:
      if py._cobj == c:
         return
      if py._own:
         del py._cobj
   py._cobj = c
   py._own = own


