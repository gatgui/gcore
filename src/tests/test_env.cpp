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

#include <gcore/env.h>
#include <gcore/path.h>

bool PathCallback(const gcore::Path &path)
{
   std::cout << "  \"" << path.fullname() << "\"" << std::endl;
   return true;
}

int main(int, char**)
{
   gcore::Env::Dict all;
   gcore::Env::Dict::iterator it;
   
   gcore::Env env;
   
   std::cout << "Username(): " << gcore::Env::Username() << std::endl;
   std::cout << "Hostname(): " << gcore::Env::Hostname() << std::endl;
   
   env.asDict(all);
   it = all.begin();
   while (it != all.end())
   {
      std::cout << it->first << "=" << it->second << std::endl;
      ++it;
   }
   
   std::cout << "Check env var BSROOT..." << std::endl;
   
   gcore::String bsroot = "Z:/ve/home/GaetanG/dev/deploy/projects";
   
   if (env.isSet("BSROOT"))
   {
      std::cout << "BSROOT already set: \"" << env.get("BSROOT") << "\"" << std::endl;
      env.set("BSROOT", bsroot, true);
   }
   else
   {
      env.set("BSROOT", bsroot, false);
   }
   std::cout << "BSROOT = \"" << env.get("BSROOT") << "\"" << std::endl;
   
   std::cout << "PATH content..." << std::endl;
   gcore::Env::ForEachInPathFunc cb;
   gcore::Bind(PathCallback, cb);
   gcore::Env::ForEachInPath("PATH", cb);
   
   std::cout << "PATH content... (using PathList)" << std::endl;
   gcore::PathList pl;
   gcore::Env::ListPaths("PATH", pl);
   std::cout << pl << std::endl;
   
   return 0;
}

