/*
MIT License

Copyright (c) 2009 Gaetan Guidet

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

#ifndef __gcore_tokenizer_h_
#define __gcore_tokenizer_h_

#include <gcore/config.h>

namespace gcore {
  
  class GCORE_API Tokenizer {
    
    public:

      Tokenizer();
      Tokenizer(const char *dlims);
      ~Tokenizer();

      void setString(const char *s);
      void setDelimiters(const char *d);
      void addDelimiter(char d);

      bool isDelim(char d);

      bool next(void);

      const char* getToken(void) const;

    private:

      char *mStr;
      char *mToken;
      char *mNextToken;
      char mDelims[256];
      unsigned int mNumDelim;
  };
  
  
}



#endif
