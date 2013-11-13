/*

Copyright (C) 2013  Gaetan Guidet

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

#include <gcore/base64.h>

#define MASK0  0x00FC0000
#define MASK1  0x0003F000
#define MASK2  0x00000FC0
#define MASK3  0x0000003F

#define SHIFT0 18
#define SHIFT1 12
#define SHIFT2 6
#define SHIFT3 0

namespace gcore {

static const char* gsEncTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static char* gsDecTable = NULL;

class _Base64Init {
  public:
    _Base64Init() {
      gsDecTable = new char[256];
      for (int i=0; i<64; ++i) {
        gsDecTable[int(gsEncTable[i])] = char(i);
      }
    }
    ~_Base64Init() {
      delete[] gsDecTable;
    }
};

static _Base64Init _b64init;

// ---

size_t Base64::EncodeLength(size_t inlen) {
  return (4 * ((inlen / 3) + (inlen % 3 ? 1 : 0)));
}

std::string Base64::Encode(const void *data, size_t len) {
  if (!data) {
    return "";
  }
  
  std::string out = "";
  unsigned long tmp;
  size_t p = 0;
  const unsigned char *bytes = (const unsigned char*) data;
  
  while ((len - p) >= 3) {
    tmp = (bytes[p] << 16) | (bytes[p+1] << 8) | bytes[p+2];
    out.push_back(gsEncTable[(tmp & MASK0) >> SHIFT0]);
    out.push_back(gsEncTable[(tmp & MASK1) >> SHIFT1]);
    out.push_back(gsEncTable[(tmp & MASK2) >> SHIFT2]);
    out.push_back(gsEncTable[(tmp & MASK3) >> SHIFT3]);
    p += 3;
  }
  
  if ((len - p) == 2) {
    tmp = (bytes[p] << 16) | (bytes[p+1] << 8);
    out.push_back(gsEncTable[(tmp & MASK0) >> SHIFT0]);
    out.push_back(gsEncTable[(tmp & MASK1) >> SHIFT1]);
    out.push_back(gsEncTable[(tmp & MASK2) >> SHIFT2]);
    out.push_back('=');
    
  } else if ((len - p) == 1) {
    tmp = (bytes[p] << 16);
    out.push_back(gsEncTable[(tmp & MASK0) >> SHIFT0]);
    out.push_back(gsEncTable[(tmp & MASK1) >> SHIFT1]);
    out.push_back('=');
    out.push_back('=');
  }
  
  return out;
}

std::string Base64::Encode(const std::string &in) {
  return Encode(in.c_str(), in.length());
}

size_t Base64::DecodeLength(const char *in, size_t inlen) {
  if (inlen == 0 || (inlen % 4) != 0) {
    return 0;
  } else {
    return (3 * (inlen / 4) - (in[inlen-1] == '=' ? (in[inlen-2] == '=' ? 2 : 1) : 0));
  }
}

size_t Base64::Decode(const std::string &in, void *data, size_t maxlen) {
  size_t declen = DecodeLength(in.c_str(), in.length());
  
  if (!data || maxlen < declen || declen == 0) {
    return 0;
  }
  
  unsigned long tmp;
  int npad;
  size_t p = 0;
  size_t outp = 0;
  size_t len = in.length();
  unsigned char *out = (unsigned char*) data;
  
  while ((len - p) > 0) {
    tmp = 0;
    npad = 0;
    
    for (int i=0, o=18; i<4; ++i, o-=6) {
      char c = in[p+i];
      if (c != '=') {
        tmp |= (gsDecTable[int(c)] << o);
      } else {
        ++npad;
      }
    }
    
    if (npad == 0) {
      out[outp++] = (tmp & 0x00FF0000) >> 16;
      out[outp++] = (tmp & 0x0000FF00) >> 8;
      out[outp++] = (tmp & 0x000000FF);
      
    } else if (npad == 1) {
      out[outp++] = (tmp & 0x00FF0000) >> 16;
      out[outp++] = (tmp & 0x0000FF00) >> 8;
      
    } else if (npad == 2) {
      out[outp++] = (tmp & 0x00FF0000) >> 16;
      
    } else {
      return 0;
    }
    
    p += 4;
  }
  
  return outp;
}

std::string Base64::Decode(const std::string &in) {
  size_t len = DecodeLength(in.c_str(), in.length());
  if (len > 0) {
    char *buffer = (char*) malloc(len+1);
    if (Decode(in, buffer, len) != 0) {
      buffer[len] = '\0';
      std::string rv(buffer);
      return rv;
    } else {
      free(buffer);
      return "";
    }
  } else {
    return "";
  }
}

}
