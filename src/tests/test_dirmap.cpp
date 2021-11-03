/*
MIT License

Copyright (c) 2010 Gaetan Guidet

This file is part of gcore.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <gcore/dirmap.h>

int main(int argc, char **argv) {
  
  if (argc <= 1) {
    gcore::Dirmap::AddMapping("z:/home", "/Users");
    gcore::Dirmap::AddMapping("z:/work", "/Work");
    gcore::Dirmap::AddMapping("/mnt/export/ifs/marza", "/Users"); // only works on linux
    gcore::Dirmap::AddMapping("C:/Users/Shared/marza", "Z:/home"); // only works on windows
    gcore::Dirmap::WriteMappingsToFile("./dirmap.cfg");
  } else {
    gcore::Dirmap::ReadMappingsFromFile(gcore::Path(argv[1]));
  }
  
  gcore::String path0 = "/Users/gatgui/dev/temp";
  gcore::String path1 = "/Work/projectA/data/";
  gcore::String path2 = "z:\\home\\gatgui/.preferences";
  gcore::String path3 = "z:/WORK/ProjectA/scenes\\";
  gcore::String path4 = "/mnt/export/ifs/marza/home/me/hello.txt";
  gcore::String path5 = "C:/Users/Shared/marza/home/me/goodbye.txt";
  
  std::cout << path0 << " -> " << gcore::Dirmap::Map(path0) << std::endl;
  std::cout << path1 << " -> " << gcore::Dirmap::Map(path1) << std::endl;
  std::cout << path2 << " -> " << gcore::Dirmap::Map(path2) << std::endl;
  std::cout << path3 << " -> " << gcore::Dirmap::Map(path3) << std::endl;
  std::cout << path4 << " -> " << gcore::Dirmap::Map(path4) << std::endl;
  std::cout << path5 << " -> " << gcore::Dirmap::Map(path5) << std::endl;
  
  gcore::Path npath("/Users/gatgui/music");
  std::cout << npath.fullname() << std::endl;
  
  gcore::Path wpath("z:\\home\\GatGui/Music");
  std::cout << wpath.fullname() << std::endl;
  
  return 0;
}
