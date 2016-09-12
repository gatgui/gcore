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

#ifndef __gcore_base64_h_
#define __gcore_base64_h_

#include <gcore/config.h>
#include <gcore/string.h>

namespace gcore {
  
  namespace base64 {
    
    GCORE_API size_t EncodeLength(size_t inlen);
    
    GCORE_API bool Encode(const void *in, size_t inlen, char *out, size_t outlen);
    GCORE_API char* Encode(const void *in, size_t inlen, size_t &outlen);
    GCORE_API bool Encode(const void *in, size_t inlen, gcore::String &out);
    GCORE_API bool Encode(const gcore::String &in, char *out, size_t outlen);
    GCORE_API char* Encode(const gcore::String &in, size_t &outlen);
    GCORE_API bool Encode(const gcore::String &in, gcore::String &out);
    GCORE_API gcore::String Encode(const void *data, size_t len);
    GCORE_API gcore::String Encode(const gcore::String &in);
    
    // ---

    GCORE_API size_t DecodeLength(const char *in, size_t len);

    GCORE_API bool Decode(const char *in, size_t len, void *out, size_t outlen);
    GCORE_API void* Decode(const char *in, size_t len, size_t &outlen);
    GCORE_API bool Decode(const char *in, size_t len, gcore::String &out);
    GCORE_API bool Decode(const gcore::String &in, void *out, size_t outlen);
    GCORE_API void* Decode(const gcore::String &in, size_t &outlen);
    GCORE_API bool Decode(const gcore::String &in, gcore::String &out);
    GCORE_API gcore::String Decode(const char *in, size_t len);
    GCORE_API gcore::String Decode(const gcore::String &in);
  }
}

#endif
