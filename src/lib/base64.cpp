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

static bool _Encode(const void *data, size_t len, char *&out, size_t &outlen) {
  if (!data) {
    if (!out) {
      outlen = 0;
    }
    return false;
  }
  
  size_t enclen = Base64::EncodeLength(len);
  if (enclen == 0) {
    if (!out) {
      outlen = 0;
    }
    return false;
  }

  if (!out) {
    outlen = enclen;
    out = (char*) malloc(outlen+1);
    out[outlen] = '\0';
    
  } else if (outlen < enclen) {
    return false;

  }

  const unsigned char *bytes = (const unsigned char*) data;
  unsigned long tmp;
  size_t outp = 0;
  size_t p = 0;
  
  while ((len - p) >= 3) {
    tmp = (bytes[p] << 16) | (bytes[p+1] << 8) | bytes[p+2];
    out[outp++] = gsEncTable[(tmp & MASK0) >> SHIFT0];
    out[outp++] = gsEncTable[(tmp & MASK1) >> SHIFT1];
    out[outp++] = gsEncTable[(tmp & MASK2) >> SHIFT2];
    out[outp++] = gsEncTable[(tmp & MASK3) >> SHIFT3];
    p += 3;
  }
  
  if ((len - p) == 2) {
    tmp = (bytes[p] << 16) | (bytes[p+1] << 8);
    out[outp++] = gsEncTable[(tmp & MASK0) >> SHIFT0];
    out[outp++] = gsEncTable[(tmp & MASK1) >> SHIFT1];
    out[outp++] = gsEncTable[(tmp & MASK2) >> SHIFT2];
    out[outp++] = '=';
    
  } else if ((len - p) == 1) {
    tmp = (bytes[p] << 16);
    out[outp++] = gsEncTable[(tmp & MASK0) >> SHIFT0];
    out[outp++] = gsEncTable[(tmp & MASK1) >> SHIFT1];
    out[outp++] = '=';
    out[outp++] = '=';
  }

  return true;
}

bool Base64::Encode(const void *data, size_t len, char *out, size_t outlen) {
  return _Encode(data, len, out, outlen);
}

char* Base64::Encode(const void *data, size_t len, size_t &outlen) {
  char *out = 0;
  outlen = 0;
  if (!_Encode(data, len, out, outlen)) {
    return 0;
  } else {
    return out;
  }
}

bool Base64::Encode(const void *data, size_t len, std::string &out) {
  size_t outlen = EncodeLength(len);
  out.resize(outlen);
  char *outbytes = (char*) out.data();
  if (!_Encode(data, len, outbytes, outlen)) {
    out = "";
    return false;
  } else {
    return true;
  }
}

bool Base64::Encode(const std::string &in, char *out, size_t outlen) {
  return _Encode(in.c_str(), in.length(), out, outlen);
}

char* Base64::Encode(const std::string &in, size_t &outlen) {
  return Encode(in.c_str(), in.length(), outlen);
}

bool Base64::Encode(const std::string &in, std::string &out) {
  return Encode(in.c_str(), in.length(), out);
}

std::string Base64::Encode(const void *data, size_t len) {
  std::string rv;
  Encode(data, len, rv);
  return rv;
}

std::string Base64::Encode(const std::string &in) {
  return Encode(in.c_str(), in.length());
}

// ---

size_t Base64::DecodeLength(const char *in, size_t len) {
  if (!in || len == 0 || (len % 4) != 0) {
    return 0;
  } else {
    return (3 * (len / 4) - (in[len-1] == '=' ? (in[len-2] == '=' ? 2 : 1) : 0));
  }
}

static bool _Decode(const char *in, size_t len, void* &out, size_t &outlen) {
  size_t declen = Base64::DecodeLength(in, len);
  if (declen == 0) {
    if (!out) {
      outlen = 0;
    }
    return false;
  }

  bool allocated = false;

  if (!out) {
    outlen = declen;
    out = malloc(outlen);
    allocated = true;

  } else if (outlen < declen) {
    return false;
  }
  
  unsigned long tmp;
  int npad;
  size_t p = 0;
  size_t outp = 0;
  unsigned char *bytes = (unsigned char*) out;
  
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
      bytes[outp++] = (tmp & 0x00FF0000) >> 16;
      bytes[outp++] = (tmp & 0x0000FF00) >> 8;
      bytes[outp++] = (tmp & 0x000000FF);
      
    } else if (npad == 1) {
      bytes[outp++] = (tmp & 0x00FF0000) >> 16;
      bytes[outp++] = (tmp & 0x0000FF00) >> 8;
      
    } else if (npad == 2) {
      bytes[outp++] = (tmp & 0x00FF0000) >> 16;
      
    } else {
      if (allocated) {
        free(out);
        out = 0;
        outlen = 0;
      }
      return false;
    }
    
    p += 4;
  }
  
  return true;
}

bool Base64::Decode(const char *in, size_t len, void *out, size_t outlen) {
  return _Decode(in, len, out, outlen);
}

void* Base64::Decode(const char *in, size_t len, size_t &outlen) {
  void *out = 0;
  outlen = 0;
  if (!_Decode(in, len, out, outlen)) {
    return 0;
  } else {
    return out;
  }
}

bool Base64::Decode(const char *in, size_t len, std::string &out) {
  size_t outlen = DecodeLength(in, len);
  out.resize(outlen);
  void *outbytes = (void*) out.data();
  if (!_Decode(in, len, outbytes, outlen)) {
    out = "";
    return false;
  } else {
    return true;
  }
}

bool Base64::Decode(const std::string &in, void *out, size_t outlen) {
  return _Decode(in.c_str(), in.length(), out, outlen);
}

void* Base64::Decode(const std::string &in, size_t &outlen) {
  return Decode(in.c_str(), in.length(), outlen);
}

bool Base64::Decode(const std::string &in, std::string &out) {
  return Decode(in.c_str(), in.length(), out);
}

std::string Base64::Decode(const char *in, size_t len) {
  std::string rv;
  Decode(in, len, rv);
  return rv;
}

std::string Base64::Decode(const std::string &in) {
  return Decode(in.c_str(), in.length());
}

}
