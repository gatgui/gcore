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
#include <string>

namespace gcore {
  
  class GCORE_API Base64 {
    public:
    
      static size_t EncodeLength(size_t inlen);
      
      static bool Encode(const void *in, size_t inlen, char *out, size_t outlen);
      static char* Encode(const void *in, size_t inlen, size_t &outlen);
      static bool Encode(const void *in, size_t inlen, std::string &out);
      static bool Encode(const std::string &in, char *out, size_t outlen);
      static char* Encode(const std::string &in, size_t &outlen);
      static bool Encode(const std::string &in, std::string &out);
      static std::string Encode(const void *data, size_t len);
      static std::string Encode(const std::string &in);
      
      // ---

      static size_t DecodeLength(const char *in, size_t len);

      static bool Decode(const char *in, size_t len, void *out, size_t outlen);
      static void* Decode(const char *in, size_t len, size_t &outlen);
      static bool Decode(const char *in, size_t len, std::string &out);
      static bool Decode(const std::string &in, void *out, size_t outlen);
      static void* Decode(const std::string &in, size_t &outlen);
      static bool Decode(const std::string &in, std::string &out);
      static std::string Decode(const char *in, size_t len);
      static std::string Decode(const std::string &in);
  };
}

#endif
