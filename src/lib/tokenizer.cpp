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

#include <gcore/tokenizer.h>

using namespace std;

namespace gcore {
  
  Tokenizer::Tokenizer()
    : mStr(0), mToken(0), mNextToken(0), mNumDelim(0) {
    memset(mDelims, 0, 256);
  }
  
  Tokenizer::Tokenizer(const char *dlims)
    : mStr(0), mToken(0), mNextToken(0), mNumDelim(0) {
    memset(mDelims, 0, 256);
    setDelimiters(dlims);
  }
  
  Tokenizer::~Tokenizer() {
    free(mStr);
  }
  
  void Tokenizer::setString(const char *s) {
    if (!s) {
      return;
    }
    if (mStr) {
      free(mStr);
      mStr = 0;
      mToken = 0;
    }
    mStr = (char*)malloc(strlen(s)+1);
    strcpy(mStr, s);
    mToken = 0;
    mNextToken = mStr;
  }
  
  void Tokenizer::setDelimiters(const char *d) {
    if (!d) {
      return;
    }
    const char *c = d;
    while (*c != '\0') {
      addDelimiter(*c);
      c++;
    }
  }
  
  void Tokenizer::addDelimiter(char d) {
    if (!isDelim(d)) {
      mDelims[mNumDelim] = d;
      mNumDelim++;
    }
  }
  
  bool Tokenizer::isDelim(char d) {
    if (d == '\0') {
      return true;
    }
    for (unsigned int i = 0; i < mNumDelim; ++i) {
      if (d == mDelims[i]) {
        return true;
      }
    }
    return false;
  }
  
  bool Tokenizer::next(void) {
    mToken = mNextToken;
    if (!mToken || (*mToken == '\0')) {
      return false;
    }
    while (isDelim(*mToken)) {
      mToken++;
    }
    char *tokenend = mToken + 1;
    while (!isDelim(*tokenend)) {
      tokenend++;
    }
    if (*tokenend == '\0') {
      mNextToken = tokenend;
    } else {
      *tokenend = '\0';
      mNextToken = tokenend + 1;
    }
    return true;
  }
  
  const char* Tokenizer::getToken(void) const {
    return mToken;
  }
  
}

