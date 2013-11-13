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
#include <string>

namespace gcore {
  
  class GCORE_API Base64 {
    public:
    
      static size_t EncodeLength(size_t inlen);
      static std::string Encode(const void *data, size_t len);
      static std::string Encode(const std::string &in);
      
      static size_t DecodeLength(const char *in, size_t inlen);
      static size_t Decode(const std::string &in, void *data, size_t maxlen);
      static std::string Decode(const std::string &in);
  };
}

#endif
