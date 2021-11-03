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

#ifndef __gcore_dirmap_h_
#define __gcore_dirmap_h_

#include <gcore/string.h>
#include <gcore/path.h>

namespace gcore {
   
  class GCORE_API Dirmap
  {
    private:
      
      static StringDict msWin2Nix;
      static StringDict msNix2Win;
      static String _Map(const String &path, StringDict *lookup);

    public:
      
      static void AddMapping(const String &from, const String &to);
      static void RemoveMapping(const String &from, const String &to);
      static void ReadMappingsFromFile(const Path &mapfile);
      static void WriteMappingsToFile(const Path &mapfile);
      static String Map(const String &path);
  };
  
}

#endif
