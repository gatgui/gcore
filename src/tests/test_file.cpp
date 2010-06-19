/*

Copyright (C) 2009  Gaetan Guidet

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
    bool FileFound(const std::string &, const std::string &name, gcore::FileType ft) {
      if (ft == gcore::FT_DIR) {
        fprintf(stderr, "Found Directory: %s\n", name.c_str());
      } else if (ft == gcore::FT_FILE) {
        fprintf(stderr, "Found File: %s\n", name.c_str());
      } else {
        fprintf(stderr, "Found: %s\n", name.c_str());
      }
      return true;
    }
};

static bool EnumModulePath(const std::string &path) {
  fprintf(stdout, "Module search path: \"%s\"\n", path.c_str());
  return true;
}

int main(int, char **) {
  
  std::string absDir0 = gcore::GetCurrentDir();
  std::string absDir1 = gcore::NormalizePath(gcore::JoinPath(absDir0, ".."));
  std::string relDir0 = "./../src/../././tests/";
  std::string relDir1 = "../include/.";
  std::string noExtAbsFile = gcore::JoinPath(absDir1, "Makefile");
  std::string extAbsFile = gcore::JoinPath(gcore::JoinPath(absDir1, "utils"), "clear_tree.rb");
  std::string noExtRelFile = "../Makefile";
  std::string extRelFile = "../utils/./makebin.rb";
  std::string unexisting = "doesnot.exists";
  
  std::cout << "absDir0: \"" << absDir0 << "\"" << std::endl;
  std::cout << "absDir1: \"" << absDir1 << "\"" << std::endl;
  std::cout << "noExtAbsFile: \"" << noExtAbsFile << "\"" << std::endl;
  std::cout << "extAbsFile: \"" << extAbsFile << "\"" << std::endl;

  if (gcore::DirExists(absDir0) == false) {
    fprintf(stderr, "Failed: DirExists: \"%s\"\n", absDir0.c_str());
  } else {
    fprintf(stdout, "Succeeded: DirExists: \"%s\"\n", absDir0.c_str());
  }
  if (gcore::DirExists(absDir1) == false) {
    fprintf(stderr, "Failed: DirExists: \"%s\"\n", absDir1.c_str());
  } else {
    fprintf(stdout, "Succeeded: DirExists: \"%s\"\n", absDir1.c_str());
  }
  if (gcore::DirExists(relDir0) == false) {
    fprintf(stderr, "Failed: DirExists: \"%s\"\n", relDir0.c_str());
  } else {
    fprintf(stdout, "Succeeded: DirExists: \"%s\"\n", relDir0.c_str());
  }
  if (gcore::DirExists(relDir1) == false) {
    fprintf(stderr, "Failed: DirExists: \"%s\"\n", relDir1.c_str());
  } else {
    fprintf(stdout, "Succeeded: DirExists: \"%s\"\n", relDir1.c_str());
  }
  if (gcore::FileExists(absDir0) == true) {
    fprintf(stderr, "Failed: !FileExists: \"%s\"\n", absDir0.c_str());
  } else {
    fprintf(stdout, "Succeeded: !FileExists: \"%s\"\n", absDir0.c_str());
  }
  if (gcore::FileExists(absDir1) == true) {
    fprintf(stderr, "Failed: !FileExists: \"%s\"\n", absDir1.c_str());
  } else {
    fprintf(stdout, "Succeeded: !FileExists: \"%s\"\n", absDir1.c_str());
  }
  if (gcore::FileExists(relDir0) == true) {
    fprintf(stderr, "Failed: !FileExists: \"%s\"\n", relDir0.c_str());
  } else {
    fprintf(stdout, "Succeeded: !FileExists: \"%s\"\n", relDir0.c_str());
  }
  if (gcore::FileExists(relDir1) == true) {
    fprintf(stderr, "Failed: !FileExists: \"%s\"\n", relDir1.c_str());
  } else {
    fprintf(stdout, "Succeeded: !FileExists: \"%s\"\n", relDir1.c_str());
  }
  
  if (gcore::DirExists(noExtAbsFile) == true) {
    fprintf(stderr, "Failed: !DirExists: \"%s\"\n", noExtAbsFile.c_str());
  } else {
    fprintf(stdout, "Succeeded: !DirExists: \"%s\"\n", noExtAbsFile.c_str());
  }
  if (gcore::DirExists(extAbsFile) == true) {
    fprintf(stderr, "Failed: !DirExists: \"%s\"\n", extAbsFile.c_str());
  } else {
    fprintf(stdout, "Succeeded: !DirExists: \"%s\"\n", extAbsFile.c_str());
  }
  if (gcore::DirExists(noExtRelFile) == true) {
    fprintf(stderr, "Failed: !DirExists: \"%s\"\n", noExtRelFile.c_str());
  } else {
    fprintf(stdout, "Succeeded: !DirExists: \"%s\"\n", noExtRelFile.c_str());
  }
  if (gcore::DirExists(extRelFile) == true) {
    fprintf(stderr, "Failed: !DirExists: \"%s\"\n", extRelFile.c_str());
  } else {
    fprintf(stdout, "Succeeded: !DirExists: \"%s\"\n", extRelFile.c_str());
  }
  if (gcore::FileExists(noExtAbsFile) == false) {
    fprintf(stderr, "Failed: FileExists: \"%s\"\n", noExtAbsFile.c_str());
  } else {
    fprintf(stdout, "Succeeded: FileExists: \"%s\"\n", noExtAbsFile.c_str());
  }
  if (gcore::FileExists(extAbsFile) == false) {
    fprintf(stderr, "Failed: FileExists: \"%s\"\n", extAbsFile.c_str());
  } else {
    fprintf(stdout, "Succeeded: FileExists: \"%s\"\n", extAbsFile.c_str());
  }
  if (gcore::FileExists(noExtRelFile) == false) {
    fprintf(stderr, "Failed: FileExists: \"%s\"\n", noExtRelFile.c_str());
  } else {
    fprintf(stdout, "Succeeded: FileExists: \"%s\"\n", noExtRelFile.c_str());
  }
  if (gcore::FileExists(extRelFile) == false) {
    fprintf(stderr, "Failed: FileExists: \"%s\"\n", extRelFile.c_str());
  } else {
    fprintf(stdout, "Succeeded: FileExists: \"%s\"\n", extRelFile.c_str());
  }

  if (gcore::FileExists(unexisting) == true) {
    fprintf(stderr, "Failed: !FileExists: \"%s\"\n", unexisting.c_str());
  } else {
    fprintf(stdout, "Succeeded: !FileExists: \"%s\"\n", unexisting.c_str());
  }
  if (gcore::DirExists(unexisting) == true) {
    fprintf(stderr, "Failed: !DirExists: \"%s\"\n", unexisting.c_str());
  } else {
    fprintf(stdout, "Succeeded: !DirExists: \"%s\"\n", unexisting.c_str());
  }
  
  if (gcore::FileExtension(noExtRelFile).length() != 0) {
    fprintf(stderr, "Failed: !FileExtension: \"%s\"\n", noExtRelFile.c_str());
  } else {
    fprintf(stdout, "Succeeded: !FileExtension: \"%s\"\n", noExtRelFile.c_str());
  }
  std::string ext = gcore::FileExtension(extRelFile);
  if (ext.length() == 0) {
    fprintf(stderr, "Failed: FileExtension: \"%s\"\n", extRelFile.c_str());
  } else {
    fprintf(stdout, "Succeeded: FileExtension: \"%s\" = \"%s\"\n", extRelFile.c_str(), ext.c_str());
  }
  
  std::string cwd = gcore::GetCurrentDir(); 
  fprintf(stdout, "CWD = \"%s\"\n", cwd.c_str());
  
  std::string path0 = gcore::JoinPath(cwd, "Makefile");
  fprintf(stdout, "\"%s\" + \"%s\" = \"%s\"\n", cwd.c_str(), "Makefile", path0.c_str());
  
  std::string path1 = gcore::JoinPath("./", "Makefile");
  fprintf(stdout, "\"%s\" + \"%s\" = \"%s\"\n", "./", "Makefile", path1.c_str());
  
  if (gcore::IsSamePath(path0, path1) == false) {
    fprintf(stderr, "Failed: IsSamePath \"%s\", \"%s\"\n", path0.c_str(), path1.c_str());
  } else {
    fprintf(stderr, "Succeeded: IsSamePath \"%s\", \"%s\"\n", path0.c_str(), path1.c_str());
  }
  
  if (gcore::IsAbsolutePath(path0) == true) {
    fprintf(stdout, "Succeeded: IsAbsolutePath: \"%s\"\n", path0.c_str());
  } else {
    fprintf(stderr, "Failed: IsAbsolutePath: \"%s\"\n", path0.c_str());
  }
  if (gcore::IsAbsolutePath(path1) == false) {
    fprintf(stdout, "Succeeded: !IsAbsolutePath: \"%s\"\n", path1.c_str());
  } else {
    fprintf(stderr, "Failed: !IsAbsolutePath: \"%s\"\n", path1.c_str());
  }
  
  fprintf(stdout, "Basename \"%s\": \"%s\"\n", path0.c_str(), gcore::Basename(path0).c_str());
  
  std::string dir0 = gcore::Dirname(gcore::Basename(path0));
  if (dir0.length() > 0) {
    fprintf(stderr, "Failed: !Dirname: \"%s\"\n", gcore::Basename(path0).c_str());
  } else {
    fprintf(stdout, "Succeeded: !Dirname: \"%s\"\n", gcore::Basename(path0).c_str());
  }
  
  std::string tmp = gcore::Basename(absDir0);
  fprintf(stdout, "Basename \"%s\": \"%s\"\n", absDir0.c_str(), tmp.c_str());
  
  tmp = gcore::Basename(absDir1);
  fprintf(stdout, "Basename \"%s\": \"%s\"\n", absDir1.c_str(), tmp.c_str());
  
  std::string dir1 = gcore::Dirname(path0);
  if (dir1.length() == 0) {
    fprintf(stderr, "Failed: Dirname: \"%s\"\n", path0.c_str());
  } else {
    fprintf(stdout, "Dirname \"%s\": \"%s\"\n", path0.c_str(), dir1.c_str());
  }
  
  dir0 = gcore::MakeAbsolutePath(path1);
  fprintf(stdout, "Abs \"%s\": \"%s\"\n", path1.c_str(), dir0.c_str());
  
  std::string path2 = gcore::NormalizePath(relDir1);
  fprintf(stdout, "Norm \"%s\": \"%s\"\n", relDir1.c_str(), path2.c_str());
  path2 = gcore::NormalizePath(relDir0);
  fprintf(stdout, "Norm \"%s\": \"%s\"\n", relDir0.c_str(), path2.c_str());
  path2 = gcore::NormalizePath(absDir0);
  fprintf(stdout, "Norm \"%s\": \"%s\"\n", absDir0.c_str(), path2.c_str());
  path2 = gcore::NormalizePath(absDir1);
  fprintf(stdout, "Norm \"%s\": \"%s\"\n", absDir1.c_str(), path2.c_str());
  path2 = gcore::NormalizePath("C:/home/projects/../music/./Placebo");
  fprintf(stdout, "Norm \"%s\": \"%s\"\n", "C:/home/projects/../music/./Placebo", path2.c_str());
  
  
  gcore::EnumEnvCallback c0;
  gcore::MakeCallback(EnumModulePath, c0);
  gcore::ForEachInEnv("PATH", c0);
  
  AClass a;
  gcore::EnumFilesCallback c1;
  gcore::MakeCallback(&a, METHOD(AClass, FileFound), c1);
  gcore::ForEachInDir(relDir1, c1, false);
  
  return 0;
}
