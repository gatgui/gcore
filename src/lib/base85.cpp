/*

Copyright (C) 2014~  Gaetan Guidet

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

#include <gcore/base85.h>
#include <iomanip>

namespace gcore {

struct CharSetData {
  std::string enc;
  unsigned char dec[256];
  bool revbytes;
  std::map<char, unsigned int> svals;
  std::map<unsigned int, char> schars;
  char padchar;
  bool rle;
  char rlemark;
  bool candup;
  
  CharSetData()
    : revbytes(false) , padchar(0), rle(false), rlemark(0), candup(false) {
    memset(dec, 0, 256);
  }
};

static CharSetData gCharSets[Base85::CS_MAX];

class _Base85Init {
  public:
    _Base85Init() {
      
      gCharSets[Base85::CS_ASCII85].enc = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstu";
      gCharSets[Base85::CS_Z85].enc = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-:+=^!/*?&<>()[]{}@%$#";
      gCharSets[Base85::CS_IPV6].enc = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!#$%&()*+-;<=>?@^_`{|}~";
      gCharSets[Base85::CS_SA].enc = "$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwx";
      
      for (int i=0; i<85; ++i) {
        for (int j=Base85::CS_ASCII85; j<Base85::CS_MAX; ++j) {
          
          char c = gCharSets[j].enc[i];
          
          gCharSets[j].dec[(int)c] = (unsigned char)i;
          
          if (c > gCharSets[j].padchar) {
            gCharSets[j].padchar = c;
          }
        }
      }
      
      // value stored here must take into account the 'revbytes' flag
      gCharSets[Base85::CS_ASCII85].svals['z'] = 0x00000000;
      gCharSets[Base85::CS_ASCII85].svals['y'] = 0x20202020;
      gCharSets[Base85::CS_ASCII85].schars[0x00000000] = 'z';
      gCharSets[Base85::CS_ASCII85].schars[0x20202020] = 'y';
      
      // float 1.0 -> 0x0000803F
      //              0x3F800000 in reversed order
      // one more trick: when encoding integers:
      //   prefix B: each value bytes (4) creates 4 output bytes
      //   prefix F: each value bytes pair (2) create 4 output bytes
      //   prefix G: as it is
      gCharSets[Base85::CS_SA].revbytes = true;
      gCharSets[Base85::CS_SA].rle = true;
      gCharSets[Base85::CS_SA].rlemark = '!';
      gCharSets[Base85::CS_SA].candup = true;
      gCharSets[Base85::CS_SA].svals['z'] = 0x00000000;
      gCharSets[Base85::CS_SA].svals['y'] = 0x3F800000;
      gCharSets[Base85::CS_SA].schars[0x00000000] = 'z';
      gCharSets[Base85::CS_SA].schars[0x3F800000] = 'y';
    }
    
    ~_Base85Init() {
    }
};

static _Base85Init _b85init;

// ---

static bool _EncodeValue(const CharSetData &cs,
                         unsigned int val, unsigned int nchars,
                         char *&out, size_t outmaxlen, size_t &outlen) {
  
  std::map<unsigned int, char>::const_iterator it = cs.schars.find(val);
  unsigned int divisor = 85 * 85 * 85 * 85;
  
  if (it != cs.schars.end()) {
    
    out[outlen++] = it->second;
  
  } else {
    
    if (outlen+nchars > outmaxlen) {
      return false;
    }
    
    for (unsigned int i=0; i<nchars; ++i) {
      out[outlen++] = cs.enc[(val / divisor) % 85];
      divisor /= 85;
    }
  }
  
  return true;
}

static unsigned int _BytesToValue(const CharSetData &cs,
                                  const unsigned char *bytes, size_t nbytes) {
  unsigned int val = 0;
  
  if (!cs.revbytes) {
    unsigned int multiplier = 256 * 256 * 256;
    for (size_t i=0; i<nbytes; ++i) {
      val += multiplier * bytes[i];
      multiplier /= 256;
    }
     
  } else {
    unsigned int multiplier = 1;
    for (size_t i=nbytes; i<4; ++i) {
      multiplier *= 256;
    }
    for (size_t i=0; i<nbytes; ++i) {
      val += multiplier * bytes[i];
      multiplier *= 256;
    }
  }
  
  return val;
}

static bool _EncodeRepeat(const CharSetData &cs,
                          unsigned int val, unsigned int count,
                          char *&out, size_t outmaxlen, size_t &outlen) {
  
  // Because of the addition of the rlemark, count must be at least 3 for standard values
  // For special values, count should be at least 8
  //   1 ('!') + 1 (special char) + 5 (count)  = 7
  
  unsigned int minrepeat = (cs.schars.find(val) == cs.schars.end() ? 3 : 8);
  
  if (count >= minrepeat) {
    
    out[outlen++] = cs.rlemark;
    
    if (outlen >= outmaxlen) {
      return false;
    }
    
    if (!_EncodeValue(cs, val, 5, out, outmaxlen, outlen)) {
      return false;
    }
    
    if (!_EncodeValue(cs, count, 5, out, outmaxlen, outlen)) {
      return false;
    }
    
    return true;
    
  } else {
    
    for (unsigned int i=0; i<count; ++i) {
      if (!_EncodeValue(cs, val, 5, out, outmaxlen, outlen)) {
        return false;
      }
    }
    
    return true;
  }
}

static bool _EncodeChunk(const CharSetData &cs,
                         const unsigned char *in, const unsigned char *inend,
                         char *&out, size_t outmaxlen, size_t &outlen,
                         unsigned int &lastval, unsigned int &repeat) {
  
  size_t nbytes = std::min<size_t>(4, inend - in);
  
  unsigned int val = _BytesToValue(cs, in, nbytes);
  
  if (cs.rle) {
    if (repeat > 0 && val == lastval) {
      ++repeat;
      return true;
      
    } else {
      if (!_EncodeRepeat(cs, lastval, repeat, out, outmaxlen, outlen)) {
        return false;
      }
      
      lastval = val;
      
      if (in+nbytes >= inend) {
        // last chunk (partial or not)
        repeat = 0;
        return _EncodeValue(cs, val, nbytes+1, out, outmaxlen, outlen);
        
      } else {
        repeat = 1;
        return true;
      }
    }
  } else {
    return _EncodeValue(cs, val, nbytes+1, out, outmaxlen, outlen);
  }
}

// If out is provided, its size should be outlen+1 (+1 for the final '\0')
static bool _Encode(const CharSetData &cs,
                    const void *data, size_t len,
                    char *&out, size_t &outlen) {
  
  if (!data) {
    if (!out) {
      outlen = 0;
    }
    return false;
  }
  
  size_t count = len / 4;
  if (len % 4 > 0) {
    ++count;
  }
  
  bool allocated = false;
  size_t _outlen = 0;
  
  if (!out) {
    // max number of bytes
    _outlen = count * 5;
    
    out = (char*) malloc(_outlen + 1);
    memset(out, 0, _outlen + 1);
    
    std::cout << "Encode: Allocated " << _outlen << " character(s) string" << std::endl;
    
    allocated = true;
    
  } else {
    if (outlen == 0) {
      return false;
    }
    
    _outlen = outlen;
  }
  
  outlen = 0;
  
  unsigned char *cur = (unsigned char*) data;
  unsigned char *last = cur + len;
  //std::map<unsigned int, char>::const_iterator it;
  unsigned int lastval = 0;
  unsigned int repeat = 0;
  
  for (unsigned int i=0; i<count && outlen<_outlen; ++i, cur+=4) {
    
    if (!_EncodeChunk(cs, cur, last, out, _outlen, outlen, lastval, repeat)) {
      outlen = _outlen;
      break;
    }
  }
  
  if (cs.rle && repeat >= 1) {
    // have a pending repeat
    if (!_EncodeRepeat(cs, lastval, repeat, out, _outlen, outlen)) {
      _outlen = outlen;
    }
  }
  
  if (outlen > _outlen) {
    outlen = 0;
    if (allocated) {
      free(out);
      out = 0;
    }
    return false;
  
  } else {
    // outlen is a string length (doesn't take final '\0' into account)
    out[outlen] = '\0';  
    return true;
  }
}

static bool _DecodeValue(const CharSetData &cs,
                         const char *&in, const char *inend,
                         unsigned int &val, size_t &nbytes) {
  
  
  std::map<char, unsigned int>::const_iterator it = cs.svals.find(*in);
    
  if (it != cs.svals.end()) {
    
    val = it->second;
    nbytes = 4;
    
    ++in;
  
  } else {
    
    unsigned int multiplier = 85 * 85 * 85 * 85;
    unsigned int i = 0;
    
    // Remaining characters count
    size_t remain = inend - in;  

    // Required output buffer length: for 5 chars, we need 4 bytes
    nbytes = 4;

    if (remain <= 1) {
      // Encoding generates at least 2 bytes for last chunk
      // (we have already tested for special characters above)
      return false;
      
    } else if (remain < 5) {
      // Less than 5 input bytes remaining
      // encoding should have generated 2 chars for 1 byte chunk
      // encoding should have generated 3 chars for 2 byte chunk
      // encoding should have generated 4 chars for 3 byte chunk
      nbytes = remain - 1;
    }
    
    val = 0;
    
    for (; i<5 && in+i<inend; ++i) {
      // Should not contain any special characters
      if (cs.svals.find(in[i]) != cs.svals.end()) {
        return false;
      }
      val += multiplier * cs.dec[(int)in[i]];
      multiplier /= 85;
    }
    
    // Pad remaining bytes to make it 5
    for (unsigned int j=i; j<5; ++j) {
      val += multiplier * cs.padchar;
      multiplier /= 85;
    }
    
    in += i;
  }
  
  return true;
}

static bool _ValueToBytes(const CharSetData &cs,
                          unsigned int val, size_t nbytes,
                          unsigned char *&out, unsigned char *outend,
                          unsigned int dup) {
  
  // Reminder: When input chunk is not a full 5 char sequence (last chunk), decoded value is padded 'right'
  if (out >= outend) {
    return false;
  }
  
  if (dup == 2) {
    
    if (nbytes >= 4) {
      if (!_ValueToBytes(cs, (val & 0x0000FFFF), 4, out, outend, 1)) {
        std::cout << "_ValueToBytes(nbytes=" << nbytes << ", dup=" << dup << "): Failed converting lower 16 bits" << std::endl;
        return false;
      }
    } else if (nbytes >= 3) {
      if (!_ValueToBytes(cs, (val & 0x0000FF00), 4, out, outend, 1)) {
        std::cout << "_ValueToBytes(nbytes=" << nbytes << ", dup=" << dup << "): Failed converting lower 8 bits" << std::endl;
        return false;
      }
    }
    
    if (nbytes >= 2) {
      if (!_ValueToBytes(cs, (val & 0xFFFF0000) >> 16, 4, out, outend, 1)) {
        std::cout << "_ValueToBytes(nbytes=" << nbytes << ", dup=" << dup << "): Failed converting higher 16 bits" << std::endl;
        return false;
      }
    } else if (nbytes >= 1) {
      if (!_ValueToBytes(cs, (val & 0xFF000000) >> 16, 4, out, outend, 1)) {
        std::cout << "_ValueToBytes(nbytes=" << nbytes << ", dup=" << dup << "): Failed converting higher 8 bits" << std::endl;
        return false;
      }
    }
    
    return true;
  
  } else if (dup == 3) {
    unsigned int nbits = nbytes * 8;
    
    if (nbits >= 30) {
      if (!_ValueToBytes(cs, (val & 0x000003FF), 4, out, outend, 1)) {
        return false;
      }
    }
    
    if (nbits >= 20) {
      if (!_ValueToBytes(cs, (val & 0x000FFC00) >> 10, 4, out, outend, 1)) {
        return false;
      }
    }
    
    if (nbits >= 10) {
      if (!_ValueToBytes(cs, (val & 0x3FF00000) >> 20, 4, out, outend, 1)) {
        return false;
      }
    }
    
    return true;
  
  } else if (dup == 4) {
    if (nbytes >= 4) {
      if (!_ValueToBytes(cs, (val & 0x000000FF), 4, out, outend, 1)) {
        return false;
      }
    }
    if (nbytes >= 3) {
      if (!_ValueToBytes(cs, (val & 0x0000FF00) >> 8, 4, out, outend, 1)) {
        return false;
      }
    }
    if (nbytes >= 2) {
      if (!_ValueToBytes(cs, (val & 0x00FF0000) >> 16, 4, out, outend, 1)) {
        return false;
      }
    }
    if (nbytes >= 1) {
      if (!_ValueToBytes(cs, (val & 0xFF000000) >> 24, 4, out, outend, 1)) {
        return false;
      }
    }
    
    return true;
  
  } else {
    if (out + nbytes > outend) {
      std::cout << "_ValueToBytes(nbytes=" << nbytes << ", dup=" << dup << "): Not enough place left (" << (outend-out) << ")" << std::endl;
      return false;
    }
    
    if (!cs.revbytes) {
      unsigned int divisor = 256 * 256 * 256;
      for (size_t j=0; j<nbytes; ++j) {
        out[j] = (val / divisor) % 256;
        divisor /= 256;
      }
      
    } else {
      unsigned int divisor = 1;
      for (size_t j=nbytes; j<4; ++j) {
        divisor *= 256;
      }
      for (size_t j=0; j<nbytes; ++j) {
        out[j] = (val / divisor) % 256;
        divisor *= 256;
      }
    }
    
    out += nbytes;
    
    return true;
  }
}

static bool _DecodeRepeat(const CharSetData &cs,
                          const char *&in, const char *inend,
                          unsigned char *&out, unsigned char *outend,
                          unsigned int dup) {

  unsigned int value = 0;
  unsigned int count = 0;
  size_t nbytes = 0;
  
  if (!_DecodeValue(cs, in, inend, value, nbytes)) {
    return false;
  }
  
  if (!_DecodeValue(cs, in, inend, count, nbytes)) {
    return false;
  }
  
  for (unsigned int i=0; i<count; ++i) {
    //if (out + dup * nbytes > outend) {
    //  return false;
    //}
    if (!_ValueToBytes(cs, value, nbytes, out, outend, dup)) {
      return false;
    }
  }
  
  return true;
}

static bool _DecodeChunk(const CharSetData &cs,
                         const char *&in, const char *inend,
                         unsigned char *&out, unsigned char *outend,
                         unsigned int dup) {
  
  if (out >= outend) {
    std::cout << "_DecodeChunk: Invalid inputs" << std::endl;
    return false;
  }
  
  while (*in == ' ' || *in == '\t' || *in == '\n' || *in == '\r') {
    ++in;
  }
  
  if (cs.rle && *in == cs.rlemark) {
    ++in;
    if (in >= inend) {
      return false;
    }
    
    return _DecodeRepeat(cs, in, inend, out, outend, dup);
    
  } else {
    
    unsigned int val = 0;
    size_t nbytes = 0;
    
    if (!_DecodeValue(cs, in, inend, val, nbytes)) {
      return false;
    }
    
    std::cout << "Decoded value: 0x" << std::setw(8) << std::setfill('0') << std::hex << val << std::dec << " [" << nbytes << " byte(s)]" << std::endl;
    
    //if (out + dup * nbytes > outend) {
    //  return false;
    //}
    
    return _ValueToBytes(cs, val, nbytes, out, outend, dup);
    
    //return true;
  }
}

static bool _Decode(const CharSetData &cs, const char *in, size_t len, void *&data, size_t &outlen, unsigned int dup) {
  std::map<char, unsigned int>::const_iterator it;
  
  // 1: ints -> ints
  // 2: shorts -> ints
  // 3: 10 bits -> ints
  // 4: bytes -> ints
  if (dup < 1 || dup > 4) {
    return false;
  }
  
  if (dup > 1 && !cs.candup) {
    return false;
  }
  
  // Input pointers
  const char *cur = in;
  const char *last = cur + len;
  
  bool allocated = false;
  
  if (!data) {
    // Allocate enough space to hold decoded data
    
    allocated = true;
    
    outlen = 0;
    size_t l = len;
    std::cout << "Input string length: " << len << std::endl;
    
    for (it=cs.svals.begin(); it!=cs.svals.end(); ++it) {
      size_t c = std::count(in, in+len, it->first);
      if (c > 0) {
        l -= c;
        //outlen += 4 * c;
        outlen += 4 * c * dup;
        std::cout << "Found " << c << " time(s) special character '" << it->first << "'" << std::endl;
        std::cout << "  Reduce string length to: " << l << std::endl;
        std::cout << "  Increased output buffer size to: " << outlen << std::endl;
      }
    }
    
    if (cs.rle) {
      
      const char *p0 = in;
      const char *p1 = strchr(p0, cs.rlemark);
      
      while (p1 != NULL) {
        std::cout << "Found repeat pattern start @" << (p1 - in) << std::endl;
        
        // remember start postiion of repeat pattern
        const char *ps = p1;
        
        ++p1;
        
        // Decode two values: the one to be repeated and the count
        unsigned int count = 0;
        size_t n = 0;
        
        if (!_DecodeValue(cs, p1, last, count, n)) {
          return false;
        }
        std::cout << "  Value to repeat: " << count << " [" << n << " byte(s)]" << std::endl;
        
        if (!_DecodeValue(cs, p1, last, count, n)) {
          return false;
        }
        std::cout << "  Repeat " << count << " time(s)" << std::endl;
        
        // p1 should point to first character after RLE pattern
        // remove characters from total length
        // Beware we may go below 0 as repeat pattern may contain special chars
        // => need to re-add those length
        for (it=cs.svals.begin(); it!=cs.svals.end(); ++it) {
          const char *p2 = strchr(ps, it->first);
          while (p2 != NULL && p2 < p1) {
            std::cout << "  Found special char '" << it->first << "' in repeat pattern" << std::endl;
            l += 1;
            //outlen -= 4;
            outlen -= 4 * dup;
            std::cout << "    Increased string length to: " << l << std::endl;
            std::cout << "    Reduced output buffer size to: " << outlen << std::endl;
            p2 = strchr(p2+1, it->first);
          }
        }
        
        n = p1 - ps;
        if (l < n) {
          std::cout << "Failed to compute required output buffer size" << std::endl;
          return false;
          
        } else {
          l -= n;
        }
        
        //outlen += 4 * count;
        outlen += 4 * count * dup;
        std::cout << "  Reduce string length to: " << l << std::endl;
        std::cout << "  Increased output buffer size to: " << outlen << std::endl;
        
        p0 = p1;
        p1 = strchr(p0, cs.rlemark);
      }
    }
    
    // For each remaining 5 char chunk -> 4 bytes
    outlen += 4 * (l / 5) * dup;
    
    // Last chunk case
    l = l % 5;
    if (l > 0) {
      // suppose last chunk encoded normally?
      std::cout << "  " << l << " remaining character(s) [expands to " << ((l - 1) * dup) << " byte(s)]" << std::endl;
      outlen += (l - 1) * dup;
    }
    
    std::cout << "Decode: Allocated " << outlen << " byte(s)" << std::endl;
    
    data = (void*) malloc(outlen);
  }
  
  unsigned char *out = (unsigned char*) data;
  unsigned char *outbeg = out;
  unsigned char *outend = out + outlen;
   
  while (cur < last) {
    if (!_DecodeChunk(cs, cur, last, out, outend, dup)) {
      outlen = 0;
      if (allocated) {
        free(data);
        data = 0;
      }
      return false;
    }
  }
  
  // adjust outlen to real byte size
  std::cout << "Final output size: " << (out - outbeg) << " (allocated " << outlen << ")" << std::endl;
  outlen = (out - outbeg);
  
  return true;
}

// ---

bool Base85::Encode(const void *in, size_t inlen, char *out, size_t outlen, CharSet cs) {
  if (cs < CS_ASCII85 || cs >= CS_MAX) {
    return false;
  }
  const CharSetData &csd = gCharSets[cs];
  
  return _Encode(csd, in, inlen, out, outlen);
}

char* Base85::Encode(const void *in, size_t inlen, size_t &outlen, CharSet cs) {
  if (cs < CS_ASCII85 || cs >= CS_MAX) {
    return false;
  }
  const CharSetData &csd = gCharSets[cs];
  
  char *out = 0;
  outlen = 0;
  
  if (!_Encode(csd, in, inlen, out, outlen)) {
    return 0;
  } else {
    return out;
  }
}

bool Base85::Encode(const void *in, size_t inlen, std::string &out, CharSet cs) {
  if (cs < CS_ASCII85 || cs >= CS_MAX) {
    return false;
  }
  const CharSetData &csd = gCharSets[cs];
  
  size_t outlen = 0;
  char *outbytes = 0;
  
  if (!_Encode(csd, in, inlen, outbytes, outlen)) {
    out = "";
    return false;
    
  } else {
    out.assign(outbytes, outlen);
    free(outbytes);
    
    return true;
  }
}

bool Base85::Encode(const std::string &in, char *out, size_t outlen, CharSet cs) {
  return Encode(in.c_str(), in.length(), out, outlen, cs);
}

char* Base85::Encode(const std::string &in, size_t &outlen, CharSet cs) {
  return Encode(in.c_str(), in.length(), outlen, cs);
}

bool Base85::Encode(const std::string &in, std::string &out, CharSet cs) {
  return Encode(in.c_str(), in.length(), out, cs);
}

std::string Base85::Encode(const void *data, size_t len, CharSet cs) {
  std::string rv;
  Encode(data, len, rv, cs);
  return rv;
}

std::string Base85::Encode(const std::string &in, CharSet cs) {
  return Encode(in.c_str(), in.length(), cs);
}

// ---


bool Base85::Decode(const char *in, size_t len, void *&out, size_t &outlen, CharSet cs, unsigned int dup) {
  if (cs < CS_ASCII85 || cs >= CS_MAX) {
    return false;
  }
  const CharSetData &csd = gCharSets[cs];
  
  return _Decode(csd, in, len, out, outlen, dup);
}

void* Base85::Decode(const char *in, size_t len, size_t &outlen, CharSet cs, unsigned int dup) {
  if (cs < CS_ASCII85 || cs >= CS_MAX) {
    return false;
  }
  const CharSetData &csd = gCharSets[cs];
  
  void *out = 0;
  outlen = 0;
  
  if (!_Decode(csd, in, len, out, outlen, dup)) {
    return 0;
  } else {
    return out;
  }
}

bool Base85::Decode(const char *in, size_t len, std::string &out, CharSet cs, unsigned int dup) {
  if (cs < CS_ASCII85 || cs >= CS_MAX) {
    return false;
  }
  const CharSetData &csd = gCharSets[cs];
  
  void *outbytes = 0;
  size_t outlen = 0;
  
  if (!_Decode(csd, in, len, outbytes, outlen, dup)) {
    out = "";
    return false;
    
  } else {
    out.assign((const char*)outbytes, outlen);
    free(outbytes);
    return true;
  }
}

bool Base85::Decode(const std::string &in, void *&out, size_t &outlen, CharSet cs, unsigned int dup) {
  if (cs < CS_ASCII85 || cs >= CS_MAX) {
    return false;
  }
  const CharSetData &csd = gCharSets[cs];
  
  return _Decode(csd, in.c_str(), in.length(), out, outlen, dup);
}

void* Base85::Decode(const std::string &in, size_t &outlen, CharSet cs, unsigned int dup) {
  return Decode(in.c_str(), in.length(), outlen, cs, dup);
}

bool Base85::Decode(const std::string &in, std::string &out, CharSet cs, unsigned int dup) {
  return Decode(in.c_str(), in.length(), out, cs, dup);
}

std::string Base85::Decode(const char *in, size_t len, CharSet cs, unsigned int dup) {
  std::string rv;
  Decode(in, len, rv, cs, dup);
  return rv;
}

std::string Base85::Decode(const std::string &in, CharSet cs, unsigned int dup) {
  return Decode(in.c_str(), in.length(), cs, dup);
}


}

