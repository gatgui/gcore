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
      /*
      Base85::Encoding encoding;
      
      encoding.charset = "xxdfdgsh";
      encoding.rlemarker = '!';
      encoding.revbytes = true;
      encoding.specials['z'] = 0;
      encoding.specials['y'] = ' ';
      
      Base85::AddEncoding("myencoding", &encoding);
      
      Base85::Encoder *enc = Base85::CreateEncoder("myencoding", true, 1);
      
      std::string out = Base85::Encode(enc, data, len);
      
      Base85::DestroyEncoder(enc);
      
      */
      
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
