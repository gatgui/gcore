cimport gcore

class dirmap(object):
   def __init__(self):
      object.__init__(self)
   
   @staticmethod
   def AddMapping(char* wpath, char* npath):
      gcore.AddMapping(gcore.String(wpath), gcore.String(npath))
   
   @staticmethod
   def RemoveMapping(char* wpath, char* npath):
      gcore.RemoveMapping(gcore.String(wpath), gcore.String(npath))
   
   @staticmethod
   def ReadMappingsFromFile(char* path):
      gcore.ReadMappingsFromFile(gcore.Path(path))
   
   @staticmethod
   def Map(char* path):
      return gcore.Map(gcore.String(path)).c_str()

