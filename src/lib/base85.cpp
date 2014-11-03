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

// ---

static Base85::Encoding Ascii85Encoding;
static Base85::Encoding Z85Encoding;
static Base85::Encoding IPV6Encoding;

static const char* GetBuiltinEncodingName(int builtinEncoding) {
  switch (builtinEncoding) {
  case Base85::Ascii85:
    return "__ascii85__";
  case Base85::Z85:
    return "__z85__";
  case Base85::IPV6:
    return "__ipv6__";
  default:
    return 0;
  }
}

struct InternalEncoding {
  std::string enc;
  unsigned char dec[256];
  bool revbytes;
  std::map<char, unsigned int> svals;
  std::map<unsigned int, char> schars;
  char padchar;
  bool rle;
  char rlemarker;
  unsigned int pack;
  
  InternalEncoding()
    : revbytes(false)
    , padchar(0)
    , rle(false)
    , rlemarker(0)
    , pack(1) {
    memset(dec, 0, 256);
  }
  
  bool init(const Base85::Encoding *encoding) {
    if (!encoding) {
      return false;
    }
    
    enc = encoding->charset;
    revbytes = encoding->revbytes;
    svals = encoding->specials;
    padchar = 0;
    rle = (encoding->rlemarker != 0);
    rlemarker = encoding->rlemarker;
    pack = encoding->pack;
    
    memset(dec, 0, 256);
    
    for (int i=0; i<85; ++i) {
      char c = enc[i];
      
      dec[(int)c] = (unsigned char)i;
      
      if (c > padchar) {
        padchar = c;
      }
    }
    
    for (std::map<char, unsigned int>::const_iterator it = svals.begin(); it != svals.end(); ++it) {
      schars[it->second] = it->first;
    }
    
    return true;
  }
};

class EncodingRegistry {
  private:
    
    typedef std::pair<const Base85::Encoding*, InternalEncoding> EncodingEntry;
    typedef std::map<std::string, EncodingEntry> EncodingMap;
  
  public:
    
    EncodingRegistry() {
      
      Ascii85Encoding.charset = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstu";
      Ascii85Encoding.revbytes = false;
      Ascii85Encoding.rlemarker = 0;
      Ascii85Encoding.pack = 1;
      Ascii85Encoding.specials['z'] = 0x00000000;
      Ascii85Encoding.specials['y'] = 0x20202020;
      
      Z85Encoding.charset = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-:+=^!/*?&<>()[]{}@%$#";
      Z85Encoding.revbytes = false;
      Z85Encoding.rlemarker = 0;
      Z85Encoding.pack = 1;
      
      IPV6Encoding.charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!#$%&()*+-;<=>?@^_`{|}~";
      IPV6Encoding.revbytes = false;
      IPV6Encoding.rlemarker = 0;
      IPV6Encoding.pack = 1;
      
      addEncoding(GetBuiltinEncodingName(Base85::Ascii85), &Ascii85Encoding);
      addEncoding(GetBuiltinEncodingName(Base85::Z85), &Z85Encoding);
      addEncoding(GetBuiltinEncodingName(Base85::IPV6), &IPV6Encoding);
    }
    
    ~EncodingRegistry() {
    }
    
    bool addEncoding(const char *name, const Base85::Encoding *enc) {
      EncodingMap::iterator it = mEncodings.find(name);
      
      if (it != mEncodings.end()) {
        return false;
      }
      
      // Validate encoding setup
      
      if (!enc->charset || strlen(enc->charset) != 85) {
        return false;
      }
      
      if (enc->rlemarker != 0 && strchr(enc->charset, enc->rlemarker) != 0) {
        return false;
      }
      
      //if (enc->pack != 1 && enc->pack != 2 && enc->pack != 4) {
      if (enc->pack < 1 || enc->pack > 4) {
        return false;
      }
      
      for (std::map<char, unsigned int>::const_iterator sit = enc->specials.begin(); sit != enc->specials.end(); ++sit) {
        if (strchr(enc->charset, sit->first) != 0) {
          return false;
        }
      }
      
      EncodingEntry &entry = mEncodings[name];
      
      entry.first = enc;
      entry.second.init(enc);
      
      return true;
    }
    
    const Base85::Encoding* getEncoding(const char *name) const {
      if (!name) {
        return 0;
      }
      EncodingMap::const_iterator it = mEncodings.find(name);
      return (it == mEncodings.end() ? 0 : it->second.first);
    }
    
    const InternalEncoding* getInternalEncoding(const char *name) const {
      if (!name) {
        return 0;
      }
      EncodingMap::const_iterator it = mEncodings.find(name);
      return (it == mEncodings.end() ? 0 : &(it->second.second));
    }
      
  private:
    
    EncodingMap mEncodings;
};

static EncodingRegistry gRegistry;

// ---

struct Base85::Encoder {
  const InternalEncoding *encoding;
  
  const unsigned char *inbeg;
  const unsigned char *inend;
  const unsigned char *in;
  
  char *outbeg;
  char *outend;
  char *out;
  
  unsigned int last; // last encoded value
  unsigned int repeat; // repeat count
  
  size_t allocated_size;
};

struct Base85::Decoder {
  const InternalEncoding *encoding;
  
  const char *inbeg;
  const char *inend;
  const char *in;
  
  unsigned char *outbeg;
  unsigned char *outend;
  unsigned char *out;
  
  size_t allocated_size;
};

// ---

const Base85::Encoding* Base85::GetEncoding(int builtinEncoding) {
  return gRegistry.getEncoding(GetBuiltinEncodingName(builtinEncoding));
}

const Base85::Encoding* Base85::GetEncoding(const char *name){
  return gRegistry.getEncoding(name);
}

bool Base85::AddEncoding(const char *name, const Base85::Encoding *encoding) {
  return gRegistry.addEncoding(name, encoding);
}

Base85::Encoder* Base85::CreateEncoder(int builtinEncoding) {
  return CreateEncoder(GetBuiltinEncodingName(builtinEncoding));
}

Base85::Encoder* Base85::CreateEncoder(const char *encodingName) {
  const InternalEncoding *encoding = gRegistry.getInternalEncoding(encodingName);
  
  if (!encoding) {
    return 0;
  }
  
  Base85::Encoder *encoder = (Base85::Encoder*) malloc(sizeof(Base85::Encoder));
  
  encoder->encoding = encoding;
  
  return encoder;
}

void Base85::DestroyEncoder(Base85::Encoder *encoder) {
  if (encoder) {
    free(encoder);
  }
}

Base85::Decoder* Base85::CreateDecoder(int builtinEncoding) {
  return CreateDecoder(GetBuiltinEncodingName(builtinEncoding));
}

Base85::Decoder* Base85::CreateDecoder(const char *encodingName) {
  const InternalEncoding *encoding = gRegistry.getInternalEncoding(encodingName);
  
  if (!encoding) {
    return 0;
  }
  
  Base85::Decoder *decoder = (Base85::Decoder*) malloc(sizeof(Base85::Decoder));
  
  decoder->encoding = encoding;
  
  return decoder;
}

void Base85::DestroyDecoder(Base85::Decoder *decoder) {
  if (decoder) {
    free(decoder);
  }
}

// ---

static bool _EncodeValue(Base85::Encoder *e, unsigned int val, unsigned int nchars) {
  
  std::map<unsigned int, char>::const_iterator it = e->encoding->schars.find(val);
  unsigned int divisor = 85 * 85 * 85 * 85;
  
  if (it != e->encoding->schars.end()) {
    
    *(e->out) = it->second;
    
    e->out += 1;
  
  } else {
    
    if (e->out + nchars > e->outend) {
      return false;
    }
    
    for (unsigned int i=0; i<nchars; ++i) {
      e->out[i] = e->encoding->enc[(val / divisor) % 85];
      divisor /= 85;
    }
    
    e->out += nchars;
  }
  
  return true;
}

static bool _BytesToValue(Base85::Encoder *e, unsigned int &outval, unsigned int &nchars) {
  static unsigned int sPackMask[4] = {0xFFFFFFFF, 0x0000FFFF, 0x000003FF, 0x000000FF};
  static unsigned int sPackBits[4] = {32, 16, 10, 8};
  
  unsigned int mask = sPackMask[e->encoding->pack - 1];
  unsigned int shift = sPackBits[e->encoding->pack - 1];
  
  unsigned int nbits = 0;
  
  unsigned int val = 0;
  unsigned int nbytes = 0;
  unsigned int p = 0;
  
  outval = 0;
  nchars = 0;
  
  for (; p<e->encoding->pack; ++p) {
    
    nbytes = std::min<unsigned int>(4, e->inend - e->in);
    
    val = 0;
    
    if (!e->encoding->revbytes) {
      unsigned int multiplier = 256 * 256 * 256;
      for (unsigned int i=0; i<nbytes; ++i) {
        val += multiplier * e->in[i];
        multiplier /= 256;
      }
      
    } else {
      unsigned int multiplier = 1;
      for (unsigned int i=nbytes; i<4; ++i) {
        multiplier *= 256;
      }
      for (unsigned int i=0; i<nbytes; ++i) {
        val += multiplier * e->in[i];
        multiplier *= 256;
      }
    }
    
    if ((val & mask) != val) {
      std::cout << "Value to encode is too big for given packing setup: " << val << " > " << mask << std::endl;
      outval = 0;
      return false;
    }
    
    outval = outval | ((val & mask) << (shift * p));
    
    nbits += shift;
    
    e->in += nbytes;
    
    if (e->in >= e->inend) {
      break;
    }
  }
  
  nbytes = (nbits / 8) + (nbits % 8 ? 1 : 0);
  
  nchars = nbytes + 1;
  
  // pad right with zeros
  while (++p < e->encoding->pack) {
    outval = outval << shift;
  }
  
  return true;
}

static bool _EncodeRepeat(Base85::Encoder *e, unsigned int val, unsigned int count) {
  
  // Because of the addition of the rlemarker, count must be at least 3 for standard values
  // For special values, count should be at least 8
  //   1 ('!') + 1 (special char) + 5 (count)  = 7
  
  // Should I take packing into account here?
  unsigned int minrepeat = (e->encoding->schars.find(val) == e->encoding->schars.end() ? 3 : 8);
  
  if (count >= minrepeat) {
    
    *(e->out++) = e->encoding->rlemarker;
    
    if (e->out >= e->outend) {
      return false;
    }
    
    if (!_EncodeValue(e, val, 5)) {
      return false;
    }
    
    if (!_EncodeValue(e, count, 5)) {
      return false;
    }
    
    return true;
    
  } else {
    
    #ifdef _DEBUG
    if (count > 1) {
      std::cout << "Value repeat count not big enough to justify repeat pattern (" << count << " < " << minrepeat << ")" << std::endl;
    }
    #endif
    
    for (unsigned int i=0; i<count; ++i) {
      if (!_EncodeValue(e, val, 5)) {
        return false;
      }
    }
    
    return true;
  }
}

static bool _EncodeChunk(Base85::Encoder *e) {
  
  unsigned int nchars = 0;
  unsigned int val = 0;
  
  if (!_BytesToValue(e, val, nchars)) {
    return false;
  }
  
  if (e->encoding->rle) {
    if (e->repeat > 0 && val == e->last) {
      e->repeat += 1;
      return true;
      
    } else {
      if (!_EncodeRepeat(e, e->last, e->repeat)) {
        return false;
      }
      
      e->last = val;
      
      if (e->in >= e->inend) {
        // last chunk (partial or not)
        e->repeat = 0;
        return _EncodeValue(e, val, nchars);
        
      } else {
        e->repeat = 1;
        return true;
      }
    }
  } else {
    e->last = val;
    return _EncodeValue(e, val, nchars);
  }
}

// If out is provided, its size should be outlen+1 (+1 for the final '\0')
static bool _Encode(Base85::Encoder *e, const void *data, size_t len, char *&out, size_t &outlen) {
  
  if (!data) {
    if (!out) {
      outlen = 0;
    }
    return false;
  }
  
  size_t _outlen = 0;
  
  if (!out) {
    size_t count = len / 4;
    
    if (len % 4 > 0) {
      ++count;
    }
    
    // Compute the maximum number of bytes required (i.e. without compression)
    _outlen = ((count / e->encoding->pack) + (count % e->encoding->pack ? 1 : 0)) * 5;
    
    out = (char*) malloc(_outlen + 1);
    memset(out, 0, _outlen + 1);
    
    #ifdef _DEBUG
    std::cout << "Encode: Allocated " << _outlen << " character(s) string" << std::endl;
    #endif
    
    e->allocated_size = _outlen + 1;
    
  } else {
    if (outlen == 0) {
      return false;
    }
    
    _outlen = outlen;
    
    e->allocated_size = 0;
  }
  
  e->inbeg = (unsigned char*) data;
  e->inend = e->inbeg + len;
  e->in = e->inbeg;
  e->last = 0;
  e->repeat = 0;
  
  e->outbeg = out;
  e->outend = out + _outlen;
  e->out = e->outbeg;
  
  while (e->in < e->inend && e->out < e->outend) {
    
    if (!_EncodeChunk(e)) {
      // Force a failure
      e->out = e->outend + 1;
      break;
    }
  }
  
  if (e->encoding->rle && e->repeat >= 1) {
    // Have a pending repeat
    if (!_EncodeRepeat(e, e->last, e->repeat)) {
      // Force a failure
      e->out = e->outend + 1;
    }
  }
  
  if (e->out > e->outend) {
    outlen = 0;
    if (e->allocated_size > 0) {
      free(out);
      out = 0;
    }
    return false;
  
  } else {
    // outlen is a string length (doesn't take final '\0' into account)
    outlen = e->out - e->outbeg;
    out[outlen] = '\0';
    return true;
  }
}

// ---

static bool _DecodeValue(Base85::Decoder *d, unsigned int &val, size_t &nbytes) {
  
  std::map<char, unsigned int>::const_iterator it = d->encoding->svals.find(*(d->in));
    
  if (it != d->encoding->svals.end()) {
    
    val = it->second;
    nbytes = 4;
    
    d->in += 1;
  
  } else {
    
    unsigned int multiplier = 85 * 85 * 85 * 85;
    unsigned int i = 0;
    
    // Remaining characters count
    size_t remain = d->inend - d->in;
    
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
    
    for (; i<5 && d->in + i < d->inend; ++i) {
      // Should not contain any special characters
      if (d->encoding->svals.find(d->in[i]) != d->encoding->svals.end()) {
        return false;
      }
      val += multiplier * d->encoding->dec[(int) d->in[i]];
      multiplier /= 85;
    }
    
    // Pad remaining bytes to make it 5
    for (unsigned int j=i; j<5; ++j) {
      val += multiplier * d->encoding->padchar;
      multiplier /= 85;
    }
    
    d->in += i;
  }
  
  return true;
}

static bool _ValueToBytes(Base85::Decoder *d, unsigned int pack, unsigned int val, size_t nbytes) {
  
  // Reminder: When input chunk is not a full 5 char sequence (last chunk), decoded value is right padded
  if (d->out >= d->outend) {
    return false;
  }
  
  if (pack == 2) {
    if (nbytes >= 4) {
      if (!_ValueToBytes(d, 1, (val & 0x0000FFFF), 4)) {
        return false;
      }
    } else if (nbytes >= 3) {
      if (!_ValueToBytes(d, 1, (val & 0x0000FF00), 4)) {
        return false;
      }
    }
    
    if (nbytes >= 2) {
      if (!_ValueToBytes(d, 1, (val & 0xFFFF0000) >> 16, 4)) {
        return false;
      }
    } else if (nbytes >= 1) {
      if (!_ValueToBytes(d, 1, (val & 0xFF000000) >> 16, 4)) {
        return false;
      }
    }
    
    return true;
  
  } else if (pack == 3) {
    unsigned int nbits = nbytes * 8;
    
    if (nbits >= 30) {
      if (!_ValueToBytes(d, 1, (val & 0x000003FF), 4)) {
        return false;
      }
    }
    
    if (nbits >= 20) {
      if (!_ValueToBytes(d, 1, (val & 0x000FFC00) >> 10, 4)) {
        return false;
      }
    }
    
    if (nbits >= 10) {
      if (!_ValueToBytes(d, 1, (val & 0x3FF00000) >> 20, 4)) {
        return false;
      }
    }
    
    return true;
    
  } else if (pack == 4) {
    if (nbytes >= 4) {
      if (!_ValueToBytes(d, 1, (val & 0x000000FF), 4)) {
        return false;
      }
    }
    if (nbytes >= 3) {
      if (!_ValueToBytes(d, 1, (val & 0x0000FF00) >> 8, 4)) {
        return false;
      }
    }
    if (nbytes >= 2) {
      if (!_ValueToBytes(d, 1, (val & 0x00FF0000) >> 16, 4)) {
        return false;
      }
    }
    if (nbytes >= 1) {
      if (!_ValueToBytes(d, 1, (val & 0xFF000000) >> 24, 4)) {
        return false;
      }
    }
    
    return true;
  
  } else {
    if (d->out + nbytes > d->outend) {
      return false;
    }
    
    if (!d->encoding->revbytes) {
      unsigned int divisor = 256 * 256 * 256;
      for (size_t j=0; j<nbytes; ++j) {
        d->out[j] = (val / divisor) % 256;
        divisor /= 256;
      }
      
    } else {
      unsigned int divisor = 1;
      for (size_t j=nbytes; j<4; ++j) {
        divisor *= 256;
      }
      for (size_t j=0; j<nbytes; ++j) {
        d->out[j] = (val / divisor) % 256;
        divisor *= 256;
      }
    }
    
    d->out += nbytes;
    
    return true;
  }
}

static bool _DecodeRepeat(Base85::Decoder *d) {
  
  unsigned int value = 0;
  unsigned int count = 0;
  size_t nbytes = 0;
  
  // Decode value to repeat
  if (!_DecodeValue(d, value, nbytes)) {
    return false;
  }
  
  // Decode count
  if (!_DecodeValue(d, count, nbytes)) {
    return false;
  }
  
  #ifdef _DEBUG
  std::cout << "Decode value " << value << " " << count << " time(s)" << std::endl;
  #endif
  
  for (unsigned int i=0; i<count; ++i) {
    if (!_ValueToBytes(d, d->encoding->pack, value, nbytes)) {
      return false;
    }
  }
  
  return true;
}

static bool _DecodeChunk(Base85::Decoder *d) {
  
  if (d->out >= d->outend) {
    return false;
  }
  
  // Skip white spaces
  int skip = 0;
  
  while (d->in[skip] == ' ' || d->in[skip] == '\t' || d->in[skip] == '\n' || d->in[skip] == '\r') {
    ++skip;
  }
  
  d->in += skip;
  
  // Check Run Length Encoding Marker
  if (d->encoding->rle && *(d->in) == d->encoding->rlemarker) {
    d->in += 1;
    
    if (d->in >= d->inend) {
      return false;
    }
    
    return _DecodeRepeat(d);
    
  } else {
    
    unsigned int val = 0;
    size_t nbytes = 0;
    
    if (!_DecodeValue(d, val, nbytes)) {
      return false;
    }
    
    return _ValueToBytes(d, d->encoding->pack, val, nbytes);
  }
}

static bool _Decode(Base85::Decoder *d, const char *in, size_t len, void *&data, size_t &outlen) {
  // Input pointers
  d->inbeg = in;
  d->inend = in + len;
  d->in = d->inbeg;
  
  if (!data) {
    std::map<char, unsigned int>::const_iterator it, itbeg, itend;
    
    // Allocate enough space to hold decoded data (may actually be bigger)
    
    outlen = 0;
    size_t l = len;
    
    #ifdef _DEBUG
    std::cout << "Input string length: " << len << std::endl;
    #endif
    
    itbeg = d->encoding->svals.begin();
    itend = d->encoding->svals.end();
    
    for (it=itbeg; it!=itend; ++it) {
      size_t c = std::count(in, in+len, it->first);
      if (c > 0) {
        l -= c;
        
        outlen += (4 * c) * d->encoding->pack;
        
        #ifdef _DEBUG
        std::cout << "Found " << c << " time(s) special character '" << it->first << "'" << std::endl;
        std::cout << "  Reduce string length to: " << l << std::endl;
        std::cout << "  Increased output buffer size to: " << outlen << std::endl;
        #endif
      }
    }
    
    if (d->encoding->rle) {
      
      const char *p0 = in;
      const char *p1 = strchr(p0, d->encoding->rlemarker);
      
      while (p1 != NULL) {
        #ifdef _DEBUG
        std::cout << "Found repeat pattern start @" << (p1 - in) << std::endl;
        #endif
        
        // remember start postiion of repeat pattern
        const char *ps = p1;
        
        ++p1;
        
        // Decode two values: the one to be repeated and the count
        unsigned int count = 0;
        size_t n = 0;
        
        d->in = p1;
        
        // Read value to repeat
        if (!_DecodeValue(d, count, n)) {
          return false;
        }
        
        // Read repeat count
        if (!_DecodeValue(d, count, n)) {
          return false;
        }
        
        // p1 should now point to first character after RLE pattern
        // Adjust total length from total length taking extra care with special character
        
        for (it=itbeg; it!=itend; ++it) {
          
          const char *p2 = strchr(ps, it->first);
          
          while (p2 != NULL && p2 < p1) {
            #ifdef _DEBUG
            std::cout << "  Found special char '" << it->first << "' in repeat pattern" << std::endl;
            #endif
            
            l += 1;
            outlen -= 4 * d->encoding->pack;
            
            #ifdef _DEBUG
            std::cout << "    Increased string length to: " << l << std::endl;
            std::cout << "    Reduced output buffer size to: " << outlen << std::endl;
            #endif
            
            p2 = strchr(p2+1, it->first);
          }
        }
        
        n = p1 - ps;
        if (l < n) {
          #ifdef _DEBUG
          std::cout << "Failed to compute required output buffer size" << std::endl;
          #endif
          
          return false;
          
        } else {
          l -= n;
        }
        
        outlen += 4 * count * d->encoding->pack;
        
        #ifdef _DEBUG
        std::cout << "  Reduce string length to: " << l << std::endl;
        std::cout << "  Increased output buffer size to: " << outlen << std::endl;
        #endif
        
        p0 = p1;
        p1 = strchr(p0, d->encoding->rlemarker);
      }
    }
    
    // For each remaining 5 char chunk -> 4 bytes
    outlen += 4 * (l / 5) * d->encoding->pack;
    
    // Last chunk case
    l = l % 5;
    if (l > 0) {
      #ifdef _DEBUG
      std::cout << "  " << l << " remaining character(s) [expands to " << ((l - 1) * d->encoding->pack) << " byte(s)]" << std::endl;
      #endif
      
      outlen += (l - 1) * d->encoding->pack;
    }
    
    #ifdef _DEBUG
    std::cout << "Decode: Allocated " << outlen << " byte(s)" << std::endl;
    #endif
    
    data = (void*) malloc(outlen);
    
    d->allocated_size = outlen;
    
  } else {
    
    d->allocated_size = 0;
  }
  
  // Reset input pointers
  d->in = d->inbeg;
  
  // Output pointers
  d->outbeg = (unsigned char*) data;
  d->outend = d->outbeg + outlen;
  d->out = d->outbeg;
   
  while (d->in < d->inend) {
    if (!_DecodeChunk(d)) {
      outlen = 0;
      if (d->allocated_size > 0) {
        free(data);
        data = 0;
      }
      return false;
    }
  }
  
  // adjust outlen to real byte size
  #ifdef _DEBUG
  std::cout << "Final output size: " << (d->out - d->outbeg) << " (allocated " << d->allocated_size << ")" << std::endl;
  #endif
  
  outlen = (d->out - d->outbeg);
  
  return true;
}

// ---

// EncodeLength?

bool Base85::Encode(Base85::Encoder *e, const void *in, size_t inlen, char *out, size_t outlen) {
  return _Encode(e, in, inlen, out, outlen);
}

char* Base85::Encode(Base85::Encoder *e, const void *in, size_t inlen, size_t &outlen) {
  char *out = 0;
  outlen = 0;
  
  if (!_Encode(e, in, inlen, out, outlen)) {
    return 0;
  } else {
    return out;
  }
}

bool Base85::Encode(Base85::Encoder *e, const void *in, size_t inlen, std::string &out) {
  size_t outlen = 0;
  char *outbytes = 0;
  
  if (!_Encode(e, in, inlen, outbytes, outlen)) {
    out = "";
    return false;
    
  } else {
    out.assign(outbytes, outlen);
    free(outbytes);
    
    return true;
  }
}

bool Base85::Encode(Base85::Encoder *e, const std::string &in, char *out, size_t outlen) {
  return Encode(e, in.c_str(), in.length(), out, outlen);
}

char* Base85::Encode(Base85::Encoder *e, const std::string &in, size_t &outlen) {
  return Encode(e, in.c_str(), in.length(), outlen);
}

bool Base85::Encode(Base85::Encoder *e, const std::string &in, std::string &out) {
  return Encode(e, in.c_str(), in.length(), out);
}

std::string Base85::Encode(Base85::Encoder *e, const void *data, size_t len) {
  std::string rv;
  if (Encode(e, data, len, rv)) {
    return rv;
  } else {
    return "";
  }
}

std::string Base85::Encode(Base85::Encoder *e, const std::string &in) {
  return Encode(e, in.c_str(), in.length());
}

// ---

// DecodeLength?

bool Base85::Decode(Base85::Decoder *d, const char *in, size_t len, void *&out, size_t &outlen) {
  return _Decode(d, in, len, out, outlen);
}

void* Base85::Decode(Base85::Decoder *d, const char *in, size_t len, size_t &outlen) {
  void *out = 0;
  outlen = 0;
  
  if (!_Decode(d, in, len, out, outlen)) {
    return 0;
  } else {
    return out;
  }
}

bool Base85::Decode(Base85::Decoder *d, const char *in, size_t len, std::string &out) {
  void *outbytes = 0;
  size_t outlen = 0;
  
  if (!_Decode(d, in, len, outbytes, outlen)) {
    out = "";
    return false;
    
  } else {
    out.assign((const char*)outbytes, outlen);
    free(outbytes);
    return true;
  }
}

bool Base85::Decode(Base85::Decoder *d, const std::string &in, void *&out, size_t &outlen) {
  return _Decode(d, in.c_str(), in.length(), out, outlen);
}

void* Base85::Decode(Base85::Decoder *d, const std::string &in, size_t &outlen) {
  return Decode(d, in.c_str(), in.length(), outlen);
}

bool Base85::Decode(Base85::Decoder *d, const std::string &in, std::string &out) {
  return Decode(d, in.c_str(), in.length(), out);
}

std::string Base85::Decode(Base85::Decoder *d, const char *in, size_t len) {
  std::string rv;
  if (Decode(d, in, len, rv)) {
    return rv;
  } else {
    return "";
  }
}

std::string Base85::Decode(Base85::Decoder *d, const std::string &in) {
  return Decode(d, in.c_str(), in.length());
}


}

