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
