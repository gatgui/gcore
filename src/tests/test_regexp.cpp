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

#include <gcore/all.h>


int main(int, char**) {
  
  gcore::String exp = "fred(?!eric)";
  gcore::Rex re0(exp);
  
  gcore::StringList strs;
  strs.push_back("frederic");
  strs.push_back("fredo");
  
  for (size_t i=0; i<strs.size(); ++i) {
    std::cout << "\"" << strs[i] << "\" =~ /" << exp << "/" << std::endl;
    if (re0.match(strs[i])) {
      std::cout << "  Matched" << std::endl;
    } else {
      std::cout << "  Not matched" << std::endl;
    }
  }
  
  gcore::Rex re1(RAW("(\d\d):(\d\d)"));
  
  //fprintf(stderr, "Error? %s\n", re1.getError(err));
  
  gcore::RexMatch md;
  
  gcore::String str = "   Time -> 10:23  ";
  
  if (re1.search(str, md, 0, 3, str.length()-3))
  {
    fprintf(stderr, "\"%s",             md.pre().c_str() );
    fprintf(stderr, "<<%s>>",           md.group(0).c_str()  );
    fprintf(stderr, "%s\"\n",           md.post().c_str());
    fprintf(stderr, "Found1: \'%s\'\n", md.group(1).c_str()  );
    fprintf(stderr, "Found2: \'%s\'\n", md.group(2).c_str()  );
    
    //fprintf(stderr, "Substitute: %s\n", re1.substitute(md, "Current time is \\1:\\2, removed \"\\`\" and \"\\'\", whole match \"\\&\"").c_str());
    fprintf(stderr, "Substitute: %s\n", re1.substitute(md, RAW("Current time is \1:\2, removed '\`' and '\'', whole match '\&'")).c_str());
  }
  else
  {
    fprintf(stderr, "Not found\n");
  }
  
  return 0;
}
