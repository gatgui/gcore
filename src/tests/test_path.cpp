/*

Copyright (C) 2009, 2010  Gaetan Guidet

This file is part of gcore.

gcore is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at
your option) any later version.

gcore is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
USA.

*/

#include <gcore/gcore.h>

class AClass {
  public:
    bool FileFound(const Path &path) {
      if (path.isDir()) {
        fprintf(stderr, "Found Directory: %s\n", path.fullname().c_str());
      } else if (path.isFile()) {
        fprintf(stderr, "Found File: %s\n", path.fullname().c_str());
      } else {
        fprintf(stderr, "Found: %s\n", path.fullname().c_str());
      }
      return true;
    }
};

int main(int, char **) {
  
  gcore::Path absDir0 = gcore::Path::GetCurrentDir();
  gcore::Path absDir1 = absDir0 + "src/..";
  gcore::Path relDir0 = gcore::Path("./include/../src/../././src/tests");
  gcore::Path relDir1 = gcore::Path("src/../include/.");
  gcore::Path noExtAbsFile = absDir1 + "SConstruct";
  gcore::Path extAbsFile = absDir1 + "include/half.h";
  gcore::Path noExtRelFile = "./SConstruct";
  gcore::Path extRelFile = "src/../include/half.h";
  gcore::Path unexisting = "doesnot.exists";
  
  
  std::cout << "absDir0: \"" << absDir0 << "\"" << std::endl;
  std::cout << "absDir1: \"" << absDir1 << "\"" << std::endl;
  std::cout << "noExtAbsFile: \"" << noExtAbsFile << "\"" << std::endl;
  std::cout << "extAbsFile: \"" << extAbsFile << "\"" << std::endl;
  
  if (absDir0.isDir() == false) {
    fprintf(stderr, "Failed: isDir: \"%s\"\n", absDir0.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: isDir: \"%s\"\n", absDir0.fullname().c_str());
  }
  if (absDir1.isDir() == false) {
    fprintf(stderr, "Failed: isDir: \"%s\"\n", absDir1.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: isDir: \"%s\"\n", absDir1.fullname().c_str());
  }
  if (relDir0.isDir() == false) {
    fprintf(stderr, "Failed: isDir: \"%s\"\n", relDir0.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: isDir: \"%s\"\n", relDir0.fullname().c_str());
  }
  if (relDir1.isDir() == false) {
    fprintf(stderr, "Failed: isDir: \"%s\"\n", relDir1.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: isDir: \"%s\"\n", relDir1.fullname().c_str());
  }
  
  if (absDir0.isFile() == true) {
    fprintf(stderr, "Failed: not isFile: \"%s\"\n", absDir0.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: not isFile: \"%s\"\n", absDir0.fullname().c_str());
  }
  if (absDir1.isFile() == true) {
    fprintf(stderr, "Failed: not isFile: \"%s\"\n", absDir1.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: not isFile: \"%s\"\n", absDir1.fullname().c_str());
  }
  if (relDir0.isFile() == true) {
    fprintf(stderr, "Failed: not isFile: \"%s\"\n", relDir0.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: not isFile: \"%s\"\n", relDir0.fullname().c_str());
  }
  if (relDir1.isFile() == true) {
    fprintf(stderr, "Failed: not isFile: \"%s\"\n", relDir1.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: not isFile: \"%s\"\n", relDir1.fullname().c_str());
  }
  
  if (noExtAbsFile.isDir() == true) {
    fprintf(stderr, "Failed: not isDir: \"%s\"\n", noExtAbsFile.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: not isDir: \"%s\"\n", noExtAbsFile.fullname().c_str());
  }
  if (extAbsFile.isDir() == true) {
    fprintf(stderr, "Failed: not isDir: \"%s\"\n", extAbsFile.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: not isDir: \"%s\"\n", extAbsFile.fullname().c_str());
  }
  if (noExtRelFile.isDir() == true) {
    fprintf(stderr, "Failed: not isDir: \"%s\"\n", noExtRelFile.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: not isDir: \"%s\"\n", noExtRelFile.fullname().c_str());
  }
  if (extRelFile.isDir() == true) {
    fprintf(stderr, "Failed: not isDir: \"%s\"\n", extRelFile.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: not isDir: \"%s\"\n", extRelFile.fullname().c_str());
  }
  if (noExtAbsFile.isFile() == false) {
    fprintf(stderr, "Failed: isFile: \"%s\"\n", noExtAbsFile.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: isFile: \"%s\"\n", noExtAbsFile.fullname().c_str());
  }
  if (extAbsFile.isFile() == false) {
    fprintf(stderr, "Failed: isFile: \"%s\"\n", extAbsFile.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: isFile: \"%s\"\n", extAbsFile.fullname().c_str());
  }
  if (noExtRelFile.isFile() == false) {
    fprintf(stderr, "Failed: isFile: \"%s\"\n", noExtRelFile.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: isFile: \"%s\"\n", noExtRelFile.fullname().c_str());
  }
  if (extRelFile.isFile() == false) {
    fprintf(stderr, "Failed: isFile: \"%s\"\n", extRelFile.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: isFile: \"%s\"\n", extRelFile.fullname().c_str());
  }
  
  if (unexisting.isFile() == true) {
    fprintf(stderr, "Failed: not isFile: \"%s\"\n", unexisting.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: not isFile: \"%s\"\n", unexisting.fullname().c_str());
  }
  if (unexisting.isDir() == true) {
    fprintf(stderr, "Failed: not isDir: \"%s\"\n", unexisting.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: not isDir: \"%s\"\n", unexisting.fullname().c_str());
  }
  
  if (noExtRelFile.getExtension().length() != 0) {
    fprintf(stderr, "Failed: getExtension: \"%s\"\n", noExtRelFile.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: getExtension: \"%s\"\n", noExtRelFile.fullname().c_str());
  }
  String ext = extRelFile.getExtension();
  if (ext.length() == 0) {
    fprintf(stderr, "Failed: getExtension: \"%s\"\n", extRelFile.fullname().c_str());
  } else {
    fprintf(stdout, "Succeeded: getExtension: \"%s\" = \"%s\"\n", extRelFile.fullname().c_str(), ext.c_str());
  }
  
  gcore::Path cwd = gcore::Path::GetCurrentDir(); 
  fprintf(stdout, "CWD = \"%s\"\n", cwd.fullname().c_str());
  
  gcore::Path path0 = cwd + "SConstruct";
  fprintf(stdout, "\"%s\" + \"%s\" = \"%s\"\n", cwd.fullname().c_str(), "SConstruct", path0.fullname().c_str());
  
  gcore::Path path1 = gcore::Path("./") + "SConstruct";
  fprintf(stdout, "\"%s\" + \"%s\" = \"%s\"\n", "./", "SConstruct", path1.fullname().c_str());
  
  if (path0 != path1) {
    fprintf(stderr, "Failed: \"%s\" == \"%s\"\n", path0.fullname().c_str(), path1.fullname().c_str());
  } else {
    fprintf(stderr, "Succeeded: \"%s\" == \"%s\"\n", path0.fullname().c_str(), path1.fullname().c_str());
  }
  
  if (path0.isAbsolute() == true) {
    fprintf(stdout, "Succeeded: isAbsolute: \"%s\"\n", path0.fullname().c_str());
  } else {
    fprintf(stderr, "Failed: isAbsolute: \"%s\"\n", path0.fullname().c_str());
  }
  if (path1.isAbsolute() == false) {
    fprintf(stdout, "Succeeded: not isAbsolute: \"%s\"\n", path1.fullname().c_str());
  } else {
    fprintf(stderr, "Failed: not isAbsolute: \"%s\"\n", path1.fullname().c_str());
  }
  
  fprintf(stdout, "Basename \"%s\": \"%s\"\n", path0.fullname().c_str(), path0.basename().c_str());
  
  String dir0 = gcore::Path(path0.basename()).dirname();
  if (dir0.length() > 0) {
    fprintf(stderr, "Failed: dirname: \"%s\"\n", path0.basename().c_str());
  } else {
    fprintf(stdout, "Succeeded: dirname: \"%s\"\n", path0.basename().c_str());
  }
  
  String tmp = absDir0.basename();
  fprintf(stdout, "Basename \"%s\": \"%s\"\n", absDir0.fullname().c_str(), tmp.c_str());
  
  tmp = absDir1.basename();
  fprintf(stdout, "Basename \"%s\": \"%s\"\n", absDir1.fullname().c_str(), tmp.c_str());
  
  String dir1 = path0.dirname();
  if (dir1.length() == 0) {
    fprintf(stderr, "Failed: dirname: \"%s\"\n", path0.fullname().c_str());
  } else {
    fprintf(stdout, "dirname \"%s\": \"%s\"\n", path0.fullname().c_str(), dir1.c_str());
  }
  
  gcore::Path path2(path1);
  path2.makeAbsolute();
  fprintf(stdout, "makeAbsolute \"%s\": \"%s\"\n", path1.fullname().c_str(), path2.fullname().c_str());
  
  path2 = relDir1;
  path2.normalize();
  
  fprintf(stdout, "normalize \"%s\": \"%s\"\n", relDir1.fullname().c_str(), path2.fullname().c_str());
  path2 = relDir0;
  path2.normalize();
  fprintf(stdout, "normalize \"%s\": \"%s\"\n", relDir0.fullname().c_str(), path2.fullname().c_str());
  path2 = absDir0;
  path2.normalize();
  fprintf(stdout, "normalize \"%s\": \"%s\"\n", absDir0.fullname().c_str(), path2.fullname().c_str());
  path2 = absDir1;
  path2.normalize();
  fprintf(stdout, "normalize \"%s\": \"%s\"\n", absDir1.fullname().c_str(), path2.fullname().c_str());
  path2 = gcore::Path("C:/home/projects/../music/./Placebo");
  path2.normalize();
  fprintf(stdout, "normalize \"%s\": \"%s\"\n", "C:/home/projects/../music/./Placebo", path2.fullname().c_str());
  
  
  AClass a;
  gcore::Path::EachFunc c1;
  gcore::Bind(&a, METHOD(AClass, FileFound), c1);
  relDir1.each(c1, true);
  
  return 0;
}
