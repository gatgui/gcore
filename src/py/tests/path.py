import gcore
import sys

class AClass(object):
   def __init__(self):
      super(AClass, self).__init__()
   
   def fileFound(self, path):
      if path.isDir():
         print("Found directory: %s" % path.fullname())
      elif path.isFile():
         print("Found file: %s" % path.fullname())
      else:
         print("Found: %s" % path.fullname())
      return True

if __name__ == "__main__":
   
   absDir0 = gcore.Path.GetCurrentDir()
   absDir1 = gcore.Path(absDir0 + "src/..")
   relDir0 = gcore.Path("./include/../src/../././src/tests")
   relDir1 = gcore.Path("src/../include/.")
   noExtAbsFile = absDir1 + "SConstruct"
   extAbsFile = absDir1 + "include/half.h"
   noExtRelFile = gcore.Path("./SConstruct")
   extRelFile = gcore.Path("src/../include/half.h")
   unexisting = gcore.Path("doesnot.exists")
   
   print("absDir0: \"%s\"" % absDir0)
   print("absDir1: \"%s\"" % absDir1)
   print("noExtAbsFile: \"%s\"" % noExtAbsFile)
   print("extAbsFile: \"%s\"" % extAbsFile)
   
   if absDir0.isDir() is False:
      print("Failed: isDir: \"%s\"" % absDir0)
   else:
      print("Succeeded: isDir: \"%s\"" % absDir0)
   
   if absDir1.isDir() is False:
      print("Failed: isDir: \"%s\"" % absDir1)
   else:
      print("Succeeded: isDir: \"%s\"" % absDir1)
   
   if relDir0.isDir() is False:
      print("Failed: isDir: \"%s\"" % relDir0)
   else:
      print("Succeeded: isDir: \"%s\"" % relDir0)
   
   if relDir1.isDir() is False:
      print("Failed: isDir: \"%s\"" % relDir1)
   else:
      print("Succeeded: isDir: \"%s\"" % relDir1)
   
   if absDir0.isFile():
      print("Failed: !isFile: \"%s\"" % absDir0)
   else:
      print("Succeeded: !isFile: \"%s\"" % absDir0)
   
   if absDir1.isFile():
      print("Failed: !isFile: \"%s\"" % absDir1)
   else:
      print("Succeeded: !isFile: \"%s\"" % absDir1)
   
   if relDir0.isFile():
      print("Failed: !isFile: \"%s\"" % relDir0)
   else:
      print("Succeeded: !isFile: \"%s\"" % relDir0)
   
   if relDir1.isFile():
      print("Failed: !isFile: \"%s\"" % relDir1)
   else:
      print("Succeeded: !isFile: \"%s\"" % relDir1)
   
   if noExtAbsFile.isDir():
      print("Failed: !isDir: \"%s\"" % noExtAbsFile)
   else:
      print("Succeeded: !isDir: \"%s\"" % noExtAbsFile)
   
   if extAbsFile.isDir():
      print("Failed: !isDir: \"%s\"" % extAbsFile)
   else:
      print("Succeeded: !isDir: \"%s\"" % extAbsFile)
   
   if noExtRelFile.isDir():
      print("Failed: !isDir: \"%s\"" % noExtRelFile)
   else:
      print("Succeeded: !isDir: \"%s\"" % noExtRelFile)
   
   if extRelFile.isDir():
      print("Failed: !isDir: \"%s\"" % extRelFile)
   else:
      print("Succeeded: !isDir: \"%s\"" % extRelFile)
   
   if noExtAbsFile.isFile() is False:
      print("Failed: isFile: \"%s\"" % noExtAbsFile)
   else:
      print("Succeeded: isFile: \"%s\"" % noExtAbsFile)
   
   if extAbsFile.isFile() is False:
      print("Failed: isFile: \"%s\"" % extAbsFile)
   else:
      print("Succeeded: isFile: \"%s\"" % extAbsFile)
   
   if noExtRelFile.isFile() is False:
      print("Failed: isFile: \"%s\"" % noExtRelFile)
   else:
      print("Succeeded: isFile: \"%s\"" % noExtRelFile)
   
   if extRelFile.isFile() is False:
      print("Failed: isFile: \"%s\"" % extRelFile)
   else:
      print("Succeeded: isFile: \"%s\"" % extRelFile)
  
   if unexisting.isFile():
      print("Failed: !isFile: \"%s\"" % unexisting)
   else:
      print("Succeeded: !isFile: \"%s\"" % unexisting)
   
   if unexisting.isDir():
      print("Failed: !isDir: \"%s\"" % unexisting)
   else:
      print("Succeeded: !isDir: \"%s\"" % unexisting)
   
   if len(noExtRelFile.getExtension()) != 0:
      print("Failed: getExtension: \"%s\"" % noExtRelFile)
   else:
      print("Succeeded: getExtension: \"%s\"" % noExtRelFile)
   
   ext = extRelFile.getExtension()
   if len(ext) == 0:
      print("Failed: getExtension: \"%s\"" % extRelFile)
   else:
      print("Succeeded: getExtension: \"%s\"" % extRelFile)
   
   cwd = gcore.Path.GetCurrentDir()
   print("CWD = \"%s\"" % cwd)
   
   path0 = cwd + "SConstruct"
   print("\"%s\" + \"%s\" = \"%s\"" % (cwd, "SConstruct", path0))
   
   path1 = gcore.Path("./") + "SConstruct"
   print("\"%s\" + \"%s\" = \"%s\"" % ("./", "SConstruct", path1))
   
   if path0 != path0:
      print("Failed: \"%s\" == \"%s\"" % (path0, path1))
   else:
      print("Succeeded: \"%s\" == \"%s\"" % (path0, path1))
   
   if path0.isAbsolute():
      print("Succeeded: isAbsolute: \"%s\"" % path0)
   else:
      print("Failed: isAbsolute: \"%s\"" % path0)
   
   if not path1.isAbsolute():
      print("Succeeded: !isAbsolute: \"%s\"" % path1)
   else:
      print("Failed: !isAbsolute: \"%s\"" % path1)
   
   print("Basename \"%s\": \"%s\"" % (path0, path0.basename()))
   
   dir0 = gcore.Path(path0.basename()).dirname()
   if len(dir0) > 0:
      print("Failed: dirname: \"%s\"" % path0.basename())
   else:
      print("Succeeded: dirname: \"%s\"" % path0.basename())
   
   tmp = absDir0.basename()
   print("Basename \"%s\": \"%s\"" % (absDir0, tmp))
   
   tmp = absDir1.basename()
   print("Basename \"%s\": \"%s\"" % (absDir1, tmp))
   
   dir1 = path0.dirname()
   if len(dir1) == 0:
      print("Failed: dirname: \"%s\"" % path0)
   else:
      print("Succeeded: dirname: \"%s\"" % path0)
   
   path2 = gcore.Path(path1)
   path2.makeAbsolute()
   print("makeAbsolute \"%s\": \"%s\"" % (path1, path2))
   
   path2 = gcore.Path(relDir1)
   path2.normalize()
   print("normalize \"%s\": \"%s\"" % (relDir1, path2))
   
   path2 = gcore.Path(relDir0)
   path2.normalize()
   print("normalize \"%s\": \"%s\"" % (relDir0, path2))
   
   path2 = gcore.Path(absDir0)
   path2.normalize()
   print("normalize \"%s\": \"%s\"" % (absDir0, path2))
   
   path2 = gcore.Path(absDir1)
   path2.normalize()
   print("normalize \"%s\": \"%s\"" % (absDir1, path2))
   
   path2 = gcore.Path("C:/home/projects/../music/./Placbo")
   path2.normalize()
   print("normalize \"%s\": \"%s\"" % ("C:/home/projects/../music/./Placbo", path2))
   
   a = AClass()
   relDir1.each(a.fileFound, True)
   
   sys.exit(0)
