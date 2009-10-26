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

      bool toString(char *s, unsigned int maxSize) const;
      bool toByte(char *b) const;
      bool toUByte(unsigned char *b) const;
      bool toShort(short *s) const;
      bool toUShort(unsigned short *s) const;
      bool toLong(long *l) const;
      bool toULong(unsigned long *l) const;
      bool toFloat(float *f) const;
      bool toDouble(double *d) const;
      bool toBool(bool *b) const;

    private:

      char *mStr;
      char *mToken;
      char *mNextToken;
      char mDelims[256];
      unsigned int mNumDelim;
  };
  
  
}



#endif
