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
  
  bool Tokenizer::toString(char *s, unsigned int maxSize) const {
    if (mToken && s) {
      if (strlen(mToken) >= maxSize) {
        return false;
      }
      strcpy(s, mToken);
      return true;
    }
    return false;
  }
  
  //value in [-128,127]
  bool Tokenizer::toByte(char *b) const {
    if (b && mToken) {
      if (sscanf(mToken, "%hhd", b) == 1) {
        return true;
      }
    }
    return false;
  }
  
  //value in [0,255]
  bool Tokenizer::toUByte(unsigned char *b) const {
    if (b && mToken) {
      if (sscanf(mToken, "%hhu", b) == 1) {
        return true;
      }
    }
    return false;
  }
  
  bool Tokenizer::toShort(short *s) const {
    if (s && mToken) {
      return (sscanf(mToken, "%hd", s) == 1);
    }
    return false;
  }
  
  bool Tokenizer::toUShort(unsigned short *s) const {
    if (s && mToken) {
      return (sscanf(mToken, "%hu", s) == 1);
    }
    return false;
  }
  
  bool Tokenizer::toLong(long *l) const {
    if (l && mToken) {
      return (sscanf(mToken, "%ld", l) == 1);
    }
    return false;
  }
  
  bool Tokenizer::toULong(unsigned long *l) const {
    if (l && mToken) {
      return (sscanf(mToken, "%lu", l) == 1);
    }
    return false;
  }
  
  bool Tokenizer::toFloat(float *f) const {
    if (f && mToken) {
      return (sscanf(mToken, "%f", f) == 1);
    }
    return false;
  }
  
  bool Tokenizer::toDouble(double *d) const {
    if (d && mToken) {
      return (sscanf(mToken, "%lf", d) == 1);
    }
    return false;
  }
  
  bool Tokenizer::toBool(bool *b) const {
    if (b && mToken) {
      *b = !strcmp(mToken, "true"); // a bit to restrictive
      return true;
    }
    return false;
  }
  
}

