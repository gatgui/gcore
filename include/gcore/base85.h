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
#include <gcore/string.h>

namespace gcore
{
   namespace base85
   {
      enum BuiltinEncoding
      {
         Ascii85 = 0,
         Z85,
         IPV6
      };
      
      struct GCORE_API Encoding
      {
         const char *charset;
         bool revbytes;
         char rlemarker; // 0 -> not supported
         unsigned int pack; // 1, 2 or 4
         std::map<char, unsigned int> specials; // values must take revbytes into account
      };
      
      struct GCORE_API Encoder;
      struct GCORE_API Decoder;
      
      GCORE_API const Encoding* GetEncoding(int builtin);
      GCORE_API const Encoding* GetEncoding(const char *name);
      GCORE_API bool AddEncoding(const char *name, const Encoding *encoding);
      
      GCORE_API Encoder* CreateEncoder(int builtinEncoding);
      GCORE_API Encoder* CreateEncoder(const char *encodingName);
      GCORE_API void DestroyEncoder(Encoder *encoder);
      
      GCORE_API Decoder* CreateDecoder(int builtinEncoding);
      GCORE_API Decoder* CreateDecoder(const char *encodingName);
      GCORE_API void DestroyDecoder(Decoder *decodec);
      
      // ---
      
      GCORE_API bool Encode(Encoder *e, const void *in, size_t inlen, char *out, size_t outlen);
      GCORE_API char* Encode(Encoder *e, const void *in, size_t inlen, size_t &outlen);
      GCORE_API bool Encode(Encoder *e, const void *in, size_t inlen, String &out);
      GCORE_API bool Encode(Encoder *e, const String &in, char *out, size_t outlen);
      GCORE_API char* Encode(Encoder *e, const String &in, size_t &outlen);
      GCORE_API bool Encode(Encoder *e, const String &in, String &out);
      GCORE_API String Encode(Encoder *e, const void *data, size_t len);
      GCORE_API String Encode(Encoder *e, const String &in);
      
      // ---
      
      GCORE_API bool Decode(Decoder *d, const char *in, size_t len, void *&out, size_t &outlen);
      GCORE_API void* Decode(Decoder *d, const char *in, size_t len, size_t &outlen);
      GCORE_API bool Decode(Decoder *d, const char *in, size_t len, String &out);
      GCORE_API bool Decode(Decoder *d, const String &in, void *&out, size_t &outlen);
      GCORE_API void* Decode(Decoder *d, const String &in, size_t &outlen);
      GCORE_API bool Decode(Decoder *d, const String &in, String &out);
      GCORE_API String Decode(Decoder *d, const char *in, size_t len);
      GCORE_API String Decode(Decoder *d, const String &in);
   }
}

#endif
