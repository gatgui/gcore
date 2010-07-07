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

