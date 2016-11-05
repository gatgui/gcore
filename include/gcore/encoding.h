/*

Copyright (C) 2016~  Gaetan Guidet

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

#ifndef __gcore_encoding_h_
#define __gcore_encoding_h_

#include <gcore/config.h>

namespace gcore
{
   enum Encoding
   {
      ASCII = 0,
      ASCII_ISO_8859_1,
      ASCII_ISO_8859_2,
      ASCII_ISO_8859_3,
      ASCII_ISO_8859_4,
      ASCII_ISO_8859_5,
      ASCII_ISO_8859_6,
      ASCII_ISO_8859_7,
      ASCII_ISO_8859_8,
      ASCII_ISO_8859_9,
      ASCII_ISO_8859_10,
      ASCII_ISO_8859_11,
      ASCII_ISO_8859_13,
      ASCII_ISO_8859_14,
      ASCII_ISO_8859_15,
      ASCII_ISO_8859_16,
      UTF_8,
      UCS_2,
      UCS_2BE = UCS_2,
      UCS_2LE,
      UTF_16,
      UTF_16BE = UTF_16,
      UTF_16LE,
      UCS_4,
      UCS_4BE = UCS_4,
      UCS_4LE,
      UTF_32,
      UTF_32BE = UTF_32,
      UTF_32LE,
      MAX_ENCODING,
      INVALID_ENCODING = MAX_ENCODING
   };
   
   GCORE_API const char* EncodingToString(Encoding e);
   GCORE_API Encoding StringToEncoding(const char *s);
   
   // ---
   
   GCORE_API bool IsBigEndian();
   GCORE_API bool IsASCII(const char *s);
   GCORE_API bool IsUTF8(const char *s);
   
   // --- Encode/Decode single code points to/from utf-8
   
   // Unicode code points are 32 bits but only values from 0x00000000 to 0x0010FFFF are used
   typedef unsigned int Codepoint;
   
   GCORE_API extern const Codepoint InvalidCodepoint;
   GCORE_API bool IsValidCodepoint(Codepoint cp);
   
   enum ASCIICodepointFormat
   {
      ACF_16, // 16 bits: \u0000
      ACF_32, // 32 bits: \U00000000
      ACF_VARIABLE // \u{000}
   };
   
   // Return 0 on error or the number of characters written to out
   GCORE_API size_t CodepointToASCII(Codepoint cp, ASCIICodepointFormat fmt, char *out, size_t outlen);
   // Return 0 on error or the number of characters read from in
   GCORE_API size_t ASCIIToCodepoint(const char *in, Codepoint &cp);
   
   // Encode a single Codepoint to utf-8
   //   Return number of bytes required to encode the given Codepoint
   //     output will be placed in out whose maximum size is specified by outlen
   //   If any error occurs, 0 is returned
   GCORE_API size_t EncodeUTF8(Codepoint cp, char *out, size_t outlen);
   // Decode a single Codepoint from utf-8 string
   //   Returns InvalidCodepoint on error
   GCORE_API Codepoint DecodeUTF8(const char *in, size_t inlen);
   
   // --- Encode/Decode strings to/from utf-8
   
   // Encode 'e' encoded string 's' to UTF-8
   GCORE_API bool EncodeUTF8(Encoding e, const char *s, std::string &out);
   GCORE_API bool EncodeUTF8(Encoding e, const char *s, size_t len, std::string &out);
   // Encode UTF-16/32 wide string to UTF-8
   GCORE_API bool EncodeUTF8(const wchar_t *s, std::string &out);
   GCORE_API bool EncodeUTF8(const wchar_t *s, size_t len, std::string &out);
   // Decode UTF-8 string to 'e' encoding
   GCORE_API bool DecodeUTF8(const char *s, Encoding e, std::string &out);
   GCORE_API bool DecodeUTF8(const char *s, size_t len, Encoding e, std::string &out);
   // Decode UTF-8 string to UTF-16/32
   GCORE_API bool DecodeUTF8(const char *s, std::wstring &out);
   GCORE_API bool DecodeUTF8(const char *s, size_t len, std::wstring &out);
   
#ifdef _WIN32
   GCORE_API extern const int CurrentCodepage;
   GCORE_API extern const int UTF8Codepage;
   
   GCORE_API bool ToWideString(int codepage, const char *s, std::wstring &out);
   GCORE_API bool ToMultiByteString(const wchar_t *ws, int codepage, std::string &out);
#endif
}

#endif
