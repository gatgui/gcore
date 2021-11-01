/*
MIT License

Copyright (c) 2013 Gaetan Guidet

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

#ifndef __gcore_base64_h_
#define __gcore_base64_h_

#include <gcore/config.h>
#include <gcore/string.h>

namespace gcore
{
   namespace base64
   {
      GCORE_API size_t EncodeLength(size_t inlen);
      
      GCORE_API bool Encode(const void *in, size_t inlen, char *out, size_t outlen);
      GCORE_API char* Encode(const void *in, size_t inlen, size_t &outlen);
      GCORE_API bool Encode(const void *in, size_t inlen, String &out);
      GCORE_API bool Encode(const String &in, char *out, size_t outlen);
      GCORE_API char* Encode(const String &in, size_t &outlen);
      GCORE_API bool Encode(const String &in, String &out);
      GCORE_API String Encode(const void *data, size_t len);
      GCORE_API String Encode(const String &in);
      
      // ---

      GCORE_API size_t DecodeLength(const char *in, size_t len);

      GCORE_API bool Decode(const char *in, size_t len, void *out, size_t outlen);
      GCORE_API void* Decode(const char *in, size_t len, size_t &outlen);
      GCORE_API bool Decode(const char *in, size_t len, String &out);
      GCORE_API bool Decode(const String &in, void *out, size_t outlen);
      GCORE_API void* Decode(const String &in, size_t &outlen);
      GCORE_API bool Decode(const String &in, String &out);
      GCORE_API String Decode(const char *in, size_t len);
      GCORE_API String Decode(const String &in);
   }
}

#endif
