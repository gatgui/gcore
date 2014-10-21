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
#include <string>

namespace gcore {
  
  class GCORE_API Base85 {
    public:
      
      enum CharSet {
        CS_ASCII85 = 0,
        CS_Z85,
        CS_IPV6,
        CS_SA, // Solid Angle arnold base85
        CS_MAX
      };
      
      // dedup=1,2,4
      static bool Encode(const void *in, size_t inlen, char *out, size_t outlen, CharSet cs=CS_ASCII85);
      static char* Encode(const void *in, size_t inlen, size_t &outlen, CharSet cs=CS_ASCII85);
      static bool Encode(const void *in, size_t inlen, std::string &out, CharSet cs=CS_ASCII85);
      static bool Encode(const std::string &in, char *out, size_t outlen, CharSet cs=CS_ASCII85);
      static char* Encode(const std::string &in, size_t &outlen, CharSet cs=CS_ASCII85);
      static bool Encode(const std::string &in, std::string &out, CharSet cs=CS_ASCII85);
      static std::string Encode(const void *data, size_t len, CharSet cs=CS_ASCII85);
      static std::string Encode(const std::string &in, CharSet cs=CS_ASCII85);
      
      // ---
      
      static bool Decode(const char *in, size_t len, void *&out, size_t &outlen, CharSet cs=CS_ASCII85, unsigned int dup=1);
      static void* Decode(const char *in, size_t len, size_t &outlen, CharSet cs=CS_ASCII85, unsigned int dup=1);
      static bool Decode(const char *in, size_t len, std::string &out, CharSet cs=CS_ASCII85, unsigned int dup=1);
      static bool Decode(const std::string &in, void *&out, size_t &outlen, CharSet cs=CS_ASCII85, unsigned int dup=1);
      static void* Decode(const std::string &in, size_t &outlen, CharSet cs=CS_ASCII85, unsigned int dup=1);
      static bool Decode(const std::string &in, std::string &out, CharSet cs=CS_ASCII85, unsigned int dup=1);
      static std::string Decode(const char *in, size_t len, CharSet cs=CS_ASCII85, unsigned int dup=1);
      static std::string Decode(const std::string &in, CharSet cs=CS_ASCII85, unsigned int dup=1);
  };
}

#endif
