cimport gcore

class Dirmap(object):
   def __init__(self):
      object.__init__(self)
   
   @classmethod
   def AddMapping(klass, char* frompath, char* topath):
      gcore.AddMapping(gcore.String(frompath), gcore.String(topath))
   
   @classmethod
   def RemoveMapping(klass, char* frompath, char* topath):
      gcore.RemoveMapping(gcore.String(frompath), gcore.String(topath))
   
   @classmethod
   def ReadMappingsFromFile(klass, char* path):
      gcore.ReadMappingsFromFile(gcore.Path(path))
   
   @classmethod
   def Map(klass, char* path):
      return gcore.Map(gcore.String(path)).c_str()

