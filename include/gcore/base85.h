/*
MIT License

Copyright (c) 2014 Gaetan Guidet

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

#ifndef __gcore_base85_h_
#define __gcore_base85_h_

#include <gcore/config.h>
#include <string>

namespace gcore {
  
  class GCORE_API Base85 {
    public:
      
      enum BuiltinEncoding {
        Ascii85 = 0,
        Z85,
        IPV6
      };
      
      struct Encoding {
        const char *charset;
        bool revbytes;
        char rlemarker; // 0 -> not supported
        unsigned int pack; // 1, 2 or 4
        std::map<char, unsigned int> specials; // values must take revbytes into account
      };
      
      struct Encoder;
      struct Decoder;
      
      static const Encoding* GetEncoding(int builtin);
      static const Encoding* GetEncoding(const char *name);
      static bool AddEncoding(const char *name, const Encoding *encoding);
      
      static Encoder* CreateEncoder(int builtinEncoding);
      static Encoder* CreateEncoder(const char *encodingName);
      static void DestroyEncoder(Encoder *encoder);
      
      static Decoder* CreateDecoder(int builtinEncoding);
      static Decoder* CreateDecoder(const char *encodingName);
      static void DestroyDecoder(Decoder *decodec);
      
      // ---
      
      static bool Encode(Encoder *e, const void *in, size_t inlen, char *out, size_t outlen);
      static char* Encode(Encoder *e, const void *in, size_t inlen, size_t &outlen);
      static bool Encode(Encoder *e, const void *in, size_t inlen, std::string &out);
      static bool Encode(Encoder *e, const std::string &in, char *out, size_t outlen);
      static char* Encode(Encoder *e, const std::string &in, size_t &outlen);
      static bool Encode(Encoder *e, const std::string &in, std::string &out);
      static std::string Encode(Encoder *e, const void *data, size_t len);
      static std::string Encode(Encoder *e, const std::string &in);
      
      // ---
      
      static bool Decode(Decoder *d, const char *in, size_t len, void *&out, size_t &outlen);
      static void* Decode(Decoder *d, const char *in, size_t len, size_t &outlen);
      static bool Decode(Decoder *d, const char *in, size_t len, std::string &out);
      static bool Decode(Decoder *d, const std::string &in, void *&out, size_t &outlen);
      static void* Decode(Decoder *d, const std::string &in, size_t &outlen);
      static bool Decode(Decoder *d, const std::string &in, std::string &out);
      static std::string Decode(Decoder *d, const char *in, size_t len);
      static std::string Decode(Decoder *d, const std::string &in);
  };
}

#endif
