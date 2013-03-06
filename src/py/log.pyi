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
   
   def selectOutputs(self, o):
      self._cobj.selectOutputs(<unsigned int>o)
   
   def selectedOutputs(self):
      return self._cobj.selectedOutputs()
   
   def setIndentLevel(self, l):
      self._cobj.setIndentLevel(<unsigned int>l)
   
   def getIndentLevel(self):
      return self._cobj.getIndentLevel()
   
   def indent(self):
      self._cobj.indent()
   
   def unIndent(self):
      self._cobj.unIndent()
   
   def setIndentWidth(self, w):
      self._cobj.setIndentWidth(<unsigned int>w)
   
   def getIndentWidth(self):
      return self._cobj.getIndentWidth()
   
   def enableColors(self, onoff):
      self._cobj.enableColors(<bint>onoff)
   
   def colorsEnabled(self):
      return self._cobj.colorsEnabled()
   
   def showTimeStamps(self, onoff):
      self._cobj.showTimeStamps(<bint>onoff)
   
   def timeStampsShown(self):
      return self._cobj.timeStampsShown()
   
   def setOutputFunc(self, func):
      if func is None:
         self._outfunc.setPyFunc(<gcore.PyObject*>self._printStdout)
      else:
         self._outfunc.setPyFunc(<gcore.PyObject*>func)
      self._outfunc.assign(deref(self._cobj))
   
