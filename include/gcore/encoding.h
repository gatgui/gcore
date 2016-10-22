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
      MAX_ENCODING
   };
   
   GCORE_API const char* EncodingString(Encoding e);
   
   GCORE_API bool IsBigEndian();
   GCORE_API bool IsUTF8(const char *s);
   
   // Encode 'e' encoded string 's' to UTF-8
   GCORE_API bool EncodeUTF8(Encoding e, const char *s, std::string &out);
   // Encode UTF-16/32 wide string to UTF-8
   GCORE_API bool EncodeUTF8(const wchar_t *s, std::string &out);
   // Decode UTF-8 string to 'e' encoding
   GCORE_API bool DecodeUTF8(const char *s, Encoding e, std::string &out);
   // Decode UTF-8 string to UTF-16/32
   GCORE_API bool DecodeUTF8(const char *s, std::wstring &out);
   
#ifdef _WIN32
   GCORE_API bool ToWideString(int codepage, const char *s, std::wstring &out);
   GCORE_API bool ToMultiByteString(const wchar_t *ws, int codepage, std::string &out);
#endif
}

#endif
