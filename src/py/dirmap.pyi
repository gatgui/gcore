cimport gcore

class Dirmap(object):
   def __init__(self):
      object.__init__(self)
   
   @classmethod
   def AddMapping(klass, char* wpath, char* npath):
      gcore.AddMapping(gcore.String(wpath), gcore.String(npath))
   
   @classmethod
   def RemoveMapping(klass, char* wpath, char* npath):
      gcore.RemoveMapping(gcore.String(wpath), gcore.String(npath))
   
   @classmethod
   def ReadMappingsFromFile(klass, char* path):
      gcore.ReadMappingsFromFile(gcore.Path(path))
   
   @classmethod
   def Map(klass, char* path):
      return gcore.Map(gcore.String(path)).c_str()

