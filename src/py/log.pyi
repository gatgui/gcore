cimport gcore

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

