// -*- coding: utf-8 -*-
#include <string>
#include <iostream>
#include <cstring>
//#include <wchar.h>

enum Encoding
{
   ASCII = 0,
   ASCII_ISO_8859_1,
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
   UTF_32LE
};

// ---

typedef unsigned char  Byte;
typedef unsigned short Byte2;
typedef unsigned int   Byte4;

typedef char Char8;

union Char16
{
   Byte b[2];
   Byte2 i;
   
   Char16() : i(0) {}
   Char16(Byte2 v) : i(v) {}
};

union Char32
{
   Byte b[4];
   Byte4 i;
   
   Char32() : i(0) {}
   Char32(Byte4 v) : i(v) {}
};

typedef unsigned int Codepoint;

// ---

static const Byte UTF8ContinuationCharBits = 0x80; // 10xx xxxx
static const Byte UTF8LeadingChar2Bits     = 0xC0; // 110x xxxx
static const Byte UTF8LeadingChar3Bits     = 0xE0; // 1110 xxxx
static const Byte UTF8LeadingChar4Bits     = 0xF0; // 1111 0xxx

static const Byte UTF8SingleCharMask       = 0x7F; // 0111 1111
static const Byte UTF8ContinuationCharMask = 0x3F; // 0011 1111
static const Byte UTF8LeadingChar2Mask     = 0x1F; // 0001 1111
static const Byte UTF8LeadingChar3Mask     = 0x0F; // 0000 1111
static const Byte UTF8LeadingChar4Mask     = 0x07; // 0000 0111

#define UTF8ContinuationCharShift 6;

Codepoint UTF8ByteRange[4][2] = {
   {0x00000000, 0x0000007F}, // 1 byte
   {0x00000080, 0x000007FF}, // 2 bytes
   {0x00000800, 0x0000FFFF}, // 3 bytes
   {0x00010000, 0x0010FFFF}  // 4 bytes
};

// ---

static const Byte4 UTF16HighSurrogateMask = 0x000FFC00;
static const Byte4 UTF16LowSurrogateMask  = 0x000003FF;

static const int UTF16BytesOrder[2][2] = {
   {0, 1},
   {1, 0} // swap
};

// ---

static const int UTF32BytesOrder[2][4] = {
   {0, 1, 2, 3},
   {3, 2, 1, 0} // swap
};

// --- Utilities ---

inline bool IsBigEndian()
{
   // If the machine the code was compiled on is big endian
   // 0x01 will be interpreted as least significant byte in c.i
   Char16 c; // = {.i=0};
   c.b[0] = 0x00;
   c.b[1] = 0x01;
   return (c.i == 0x0001);
}

// Unicode code point are 32 bits but only 0x0000 0000 -> 0x0010 FFFF are used

const Codepoint InvalidCodepoint = 0xFFFFFFFF;

inline bool IsValidCodepoint(Codepoint cp)
{
   return (cp <= UTF8ByteRange[3][1] && (cp < 0xD800 || 0xDFFF < cp));
}

inline bool IsCombiningMark(Codepoint cp)
{
   if ((0x0300 <= cp && cp <= 0x036F) || // Combining Diacritical Marks
       (0x1AB0 <= cp && cp <= 0x1AFF) || // Combining Diacritical Marks Extended
       (0x1DC0 <= cp && cp <= 0x1DFF) || // Combining Diacritical Marks Supplement
       (0x20D0 <= cp && cp <= 0x20FF) || // Combining Diacritical Marks for Symbols
       (0xFE20 <= cp && cp <= 0xFE2F))   // Combining Half Marks
   {
      return true;
   }
   else
   {
      return false;
   }
}

// --- UTF-8 Utilities ---

bool IsUTF8(const char *s)
{
   if (!s)
   {
      return false;
   }

   const Byte *bytes = (const Byte*)s;
   Codepoint cp = InvalidCodepoint;
   int num = 0, i = 0;

   while (*bytes != 0x00)
   {
      if ((*bytes & 0x80) == 0x00)
      {
         cp = (*bytes & 0x7F);
         num = 1;
      }
      else if ((*bytes & 0xE0) == 0xC0) // E0: 1110 0000, C0: 1100 0000
      {
         cp = (*bytes & 0x1F);
         num = 2;
      }
      else if ((*bytes & 0xF0) == 0xE0) // F0: 1111 0000, E0: 1110 0000
      {
         cp = (*bytes & 0x0F);
         num = 3;
      }
      else if ((*bytes & 0xF8) == 0xF0) // F8: 1111 1000, F0: 1111 0000
      {
         cp = (*bytes & 0x07);
         num = 4;
      }
      else
      {
         return false;
      }

      bytes += 1;
      for (i=1; i<num; ++i)
      {
         if ((*bytes & 0xC0) != 0x80)
         {
            return false;
         }
         cp = (cp << 6) | (*bytes & 0x3F);
         bytes += 1;
      }

      if ((cp > 0x10FFFF) ||
          ((cp >= 0xD800) && (cp <= 0xDFFF)) ||
          ((cp <= 0x007F) && (num != 1)) ||
          ((cp >= 0x0080) && (cp <= 0x07FF) && (num != 2)) ||
          ((cp >= 0x0800) && (cp <= 0xFFFF) && (num != 3)) ||
          ((cp >= 0x10000) && (cp <= 0x1FFFFF) && (num != 4)))
      {
         return false;
      }
   }

   return true;
}

inline bool IsUTF8SingleChar(char c)
{
   // ASCII bytes starts with         0xxxxxxx
   return ((c & 0x80) == 0x00);
}

inline bool IsUTF8LeadingChar(char c)
{
   // Leading multi bytes starts with 11xxxxxx
   return ((c & 0xC0) == 0xC0);
}

inline bool IsUTF8ContinuationChar(char c)
{
   // Continuation bytes starts with  10xxxxxx
   return ((c & 0xC0) == 0x80);
}

// --- Length function ---

size_t UTF8Length(const Byte *bytes)
{
   if (!bytes)
   {
      return 0;
   }
   else
   {
      const Byte *cc = bytes;
      while (*cc != 0)
      {
         ++cc;
      }
      return (cc - bytes);
   }
}

size_t UTF16Length(const Byte *bytes)
{
   if (!bytes)
   {
      return 0;
   }
   else
   {
      const Byte *cc = bytes;
      while (cc[0] != 0 || cc[1] != 0)
      {
         cc += 2;
      }
      return (cc - bytes);
   }
}

size_t UCS2Length(const Byte *bytes)
{
   return UTF16Length(bytes);
}

size_t UTF32Length(const Byte *bytes)
{
   if (!bytes)
   {
      return 0;
   }
   else
   {
      const Byte *cc = bytes;
      while (cc[0] != 0 || cc[1] != 0 || cc[2] != 0 || cc[3] != 0)
      {
         cc += 4;
      }
      return (cc - bytes);
   }
}

// --- Encode functions ---

size_t EncodeUTF8(Codepoint cp, Byte *bytes, size_t maxlen, size_t pos)
{
   if (!IsValidCodepoint(cp))
   {
      return 0;
   }
   else
   {
      if (cp <= UTF8ByteRange[0][1])
      {
         bytes[pos] = Byte(cp & UTF8SingleCharMask);
         return 1;
      }
      else if (cp <= UTF8ByteRange[1][1])
      {
         if (pos + 1 >= maxlen)
         {
            return 0;
         }
         else
         {
            bytes[pos + 1] = Byte(UTF8ContinuationCharBits | (cp & UTF8ContinuationCharMask));
            cp >>= UTF8ContinuationCharShift;
            bytes[pos] = Byte(UTF8LeadingChar2Bits | (cp & UTF8LeadingChar2Mask));
            return 2;
         }
      }
      else if (cp <= UTF8ByteRange[2][1])
      {
         if (pos + 2 >= maxlen)
         {
            return 0;
         }
         else
         {
            bytes[pos + 2] = Byte(UTF8ContinuationCharBits | (cp & UTF8ContinuationCharMask));
            cp >>= UTF8ContinuationCharShift;
            bytes[pos + 1] = Byte(UTF8ContinuationCharBits | (cp & UTF8ContinuationCharMask));
            cp >>= UTF8ContinuationCharShift;
            bytes[pos] = Byte(UTF8LeadingChar3Bits | (cp & UTF8LeadingChar3Mask));
            return 3;
         }
      }
      else
      {
         if (pos + 3 >= maxlen)
         {
            return 0;
         }
         else
         {
            bytes[pos + 3] = Byte(UTF8ContinuationCharBits | (cp & UTF8ContinuationCharMask));
            cp >>= UTF8ContinuationCharShift;
            bytes[pos + 2] = Byte(UTF8ContinuationCharBits | (cp & UTF8ContinuationCharMask));
            cp >>= UTF8ContinuationCharShift;
            bytes[pos + 1] = Byte(UTF8ContinuationCharBits | (cp & UTF8ContinuationCharMask));
            cp >>= UTF8ContinuationCharShift;
            bytes[pos] = Byte(UTF8LeadingChar4Bits | (cp & UTF8LeadingChar4Mask));
            return 4;
         }
      }
   }
}

size_t EncodeUTF16(Codepoint cp, Byte *bytes, size_t maxlen, size_t pos, bool bigendian)
{
   if (pos + 1 >= maxlen || !IsValidCodepoint(cp))
   {
      return 0;
   }
   else
   {
      const int (&order)[2] = UTF16BytesOrder[IsBigEndian() != bigendian];
      
      if (cp <= 0xD7FF)
      {
         bytes[pos + order[0]] = Byte(cp & 0x00FF);
         bytes[pos + order[1]] = Byte((cp & 0xFF00) >> 8);
         return 2;
      }
      else if (0xE000 <= cp && cp <= 0xFFFF)
      {
         bytes[pos + order[0]] = Byte(cp & 0x00FF);
         bytes[pos + order[1]] = Byte((cp & 0xFF00) >> 8);
         return 2;
      }
      else // 0x00010000 -> 0x0010FFFF
      {
         if (pos + 3 >= maxlen)
         {
            return 0;
         }
         else
         {
            cp -= 0x00010000;
            Byte2 hs = Byte2(0xD800 + ((cp & UTF16HighSurrogateMask) >> 10));
            Byte2 ls = Byte2(0xDC00 + (cp & UTF16LowSurrogateMask));
            bytes[pos + order[0]] = Byte( hs & 0x00FF );
            bytes[pos + order[1]] = Byte((hs & 0xFF00) >> 8);
            bytes[pos + 2 + order[0]] = Byte( ls & 0x00FF );
            bytes[pos + 2 + order[1]] = Byte((ls & 0xFF00) >> 8);
            return 4;
         }
      }
   }
}

size_t EncodeUCS2(Codepoint cp, Byte *bytes, size_t maxlen, size_t pos, bool bigendian)
{
   if (cp <= 0xD7FF || (0xE000 <= cp && cp <= 0xFFFF))
   {
      return EncodeUTF16(cp, bytes, maxlen, pos, bigendian);
   }
   else
   {
      return 0;
   }
}

size_t EncodeUTF32(Codepoint cp, Byte *bytes, size_t maxlen, size_t pos, bool bigendian)
{
   if (pos + 3 >= maxlen)
   {
      return 0;
   }
   else
   {
      const int (&order)[4] = UTF32BytesOrder[IsBigEndian() != bigendian];
      
      bytes[pos + order[0]] = Byte(cp & 0x000000FF);
      bytes[pos + order[1]] = Byte((cp & 0x0000FF00) >> 8);
      bytes[pos + order[2]] = Byte((cp & 0x00FF0000) >> 16);
      bytes[pos + order[3]] = Byte((cp & 0xFF000000) >> 24);
      return 4;
   }
}

// --- Decode functions ---

Codepoint DecodeUTF8(const Byte *bytes, size_t len, size_t *_pos)
{
   if (!_pos)
   {
      return InvalidCodepoint;
   }
   
   size_t &pos = *_pos;
   Codepoint cp = 0;
   size_t n = 1;
   size_t i = 0;
   Byte b = 0;
   int shift = 0;
   
   while (i < n)
   {
      if (pos + i >= len)
      {
         std::cerr << "[UTF8] Not enough bytes available" << std::endl;
         return InvalidCodepoint;
      }
      
      b = bytes[pos + i];
      
      if (IsUTF8SingleChar(b))
      {
         cp |= b;
      }
      else if (IsUTF8LeadingChar(b))
      {
         if ((b & ~UTF8LeadingChar2Mask) == UTF8LeadingChar2Bits)
         {
            shift = UTF8ContinuationCharShift;
            cp |= (b & UTF8LeadingChar2Mask) << shift;
            n = 2;
         }
         else if ((b & ~UTF8LeadingChar3Mask) == UTF8LeadingChar3Bits)
         {
            shift = 2 * UTF8ContinuationCharShift;
            cp |= (b & UTF8LeadingChar3Mask) << shift;
            n = 3;
         }
         else if ((b & ~UTF8LeadingChar4Mask) == UTF8LeadingChar4Bits)
         {
            shift = 3 * UTF8ContinuationCharShift;
            cp |= (b & UTF8LeadingChar4Mask) << shift;
            n = 4;
         }
         else
         {
            std::cerr << "[UTF8] Invalid leading byte" << std::endl;
            return InvalidCodepoint;
         }
      }
      else
      {
         shift -= UTF8ContinuationCharShift;
         cp |= ((b & UTF8ContinuationCharMask) << shift);
      }
      
      ++i;
   }
   
   // Check that cp actually requires n bytes to be encoded in UTF-8 (overlong)
   if (cp < UTF8ByteRange[n-1][0])
   {
      std::cerr << "[UTF8] Overlong encoding" << std::endl;
      return InvalidCodepoint;
   }
   else
   {
      pos += n;
      return cp;
   }
}

inline Codepoint DecodeUTF8(const Byte *bytes, size_t len)
{
   size_t pos = 0;
   return DecodeUTF8(bytes, len, &pos);
}

Codepoint DecodeUTF16(const Byte *bytes, size_t len, size_t *_pos, bool bigendian)
{
   if (!_pos || *_pos + 1 >= len)
   {
      std::cerr << "[UTF16] Not enough bytes available (len=" << len << ", pos=";
      return InvalidCodepoint;
   }
   else
   {
      const int (&order)[2] = UTF16BytesOrder[IsBigEndian() != bigendian];
      size_t &pos = *_pos;
      
      Char16 c; // = {.i=0};
      
      c.b[order[0]] = bytes[pos];
      c.b[order[1]] = bytes[pos + 1];
      
      if (0xD800 <= c.i && c.i <= 0xDFFF)
      {
         // surrogate 
         if (c.i >= 0xDC00 || pos + 3 >= len)
         {
            // Character starting with low surrogate
            //   or
            // Not enough bytes (4 required)
            std::cerr << "[UTF16] Missing high surrogate (@" << pos << ")" << std::endl;
            return InvalidCodepoint;
         }
         else
         {
            // Invalid for UCS-2
            // Need read next, if not a surrogate, invalud char
            Char16 hs = c.i; // = {.i=c.i};
            
            c.b[order[0]] = bytes[pos + 2];
            c.b[order[1]] = bytes[pos + 3];
            
            if (c.i < 0xDC00 || 0xDFFF < c.i)
            {
               // not a low surrogate
               std::cerr << "[UTF16] Missing low surrogate (@" << (pos+2) << ")" << std::endl;
               return InvalidCodepoint;
            }
            else
            {
               pos += 4;
               return (((hs.i - 0xD800) << 10) | (c.i - 0xDC00));
            }
         }
      }
      else
      {
         pos += 2;
         return c.i;
      }
   }
}

inline Codepoint DecodeUTF16(const Byte *bytes, size_t len, bool bigendian)
{
   size_t pos = 0;
   return DecodeUTF16(bytes, len, &pos, bigendian);
}

Codepoint DecodeUCS2(const Byte *bytes, size_t len, size_t *pos, bool bigendian)
{
   Codepoint cp = DecodeUTF16(bytes, len, pos, bigendian);
   if (cp <= 0xD7FF || (0xE000 <= cp && cp <= 0xFFFF))
   {
      return cp;
   }
   else
   {
      std::cerr << "[UCS2] Invalid surrogate code point" << std::endl;
      return InvalidCodepoint;
   }
}

inline Codepoint DecodeUCS2(const Byte *bytes, size_t len, bool bigendian)
{
   size_t pos = 0;
   return DecodeUCS2(bytes, len, &pos, bigendian);
}

Codepoint DecodeUTF32(const Byte *bytes, size_t len, size_t *_pos, bool bigendian)
{
   if (!_pos || *_pos + 3 >= len)
   {
      std::cerr << "[UTF32] Not enough bytes available" << std::endl;
      return InvalidCodepoint;
   }
   else
   {
      const int (&order)[4] = UTF32BytesOrder[IsBigEndian() != bigendian];
      size_t &pos = *_pos;
      Char32 c;
      
      c.b[order[0]] = bytes[pos++];
      c.b[order[1]] = bytes[pos++];
      c.b[order[2]] = bytes[pos++];
      c.b[order[3]] = bytes[pos++];
      
      return c.i;
   }
}

inline Codepoint DecodeUTF32(const Byte *bytes, size_t len, bool bigendian)
{
   size_t pos = 0;
   return DecodeUTF32(bytes, len, &pos, bigendian);
}

// ---

bool EncodeUTF8(Encoding e, const char *s, std::string &out)
{
   if (s)
   {
      size_t i = 0;
      size_t j = 0;
      size_t k = 0;
      size_t n = 0;
      char c8 = 0;
      Char32 c32;
      const Byte *bytes = (const Byte*)s;
      unsigned int cp = InvalidCodepoint;
      
      switch (e)
      {
      case UTF_8:
         out = s;
         return true;

      case ASCII:
         n = strlen(s);
         out.resize(n);
         for (i=0; i<n; ++i)
         {
            c8 = s[i];
            if (c8 < 0)
            {
               std::cerr << "Invalid ASCII code (@" << i << "): " << c8 << " (" << int(c8) << ")" << std::endl;
               out.clear();
               return false;
            }
            out[i] = c8;
         }
         return true;
      
      case ASCII_ISO_8859_1:
         n = strlen(s);
         out.reserve(n * 2); // at most
         out.clear();
         for (i=0; i<n; ++i)
         {
            c8 = s[i];
            if (c8 < 0)
            {
               cp = (unsigned int) c8;
               j = EncodeUTF8(cp & 0x00FF, c32.b, 4, 0);
               if (j == 0)
               {
                  std::cerr << "Invalid ASCII code (@" << i << "): " << c8 << " (" << int(c8) << ")" << std::endl;
                  out.clear();
                  return false;
               }
               for (k=0; k<j; ++k)
               {
                  out.push_back(c32.b[k]);
               }
            }
            else
            {
               out.push_back(c8);
            }
         }
         return true;
      
      case UCS_2BE:
      case UCS_2LE:
         out.clear();
         n = UCS2Length(bytes);
         while (i < n)
         {
            cp = DecodeUCS2(bytes, n, &i, (e == UCS_2BE));
            if (!IsValidCodepoint(cp))
            {
               std::cerr << "Invalid code point " << cp << " (0x" << std::hex << cp << std::dec << ") in UCS-2 string" << std::endl;
               out.clear();
               return false;
            }
            j = EncodeUTF8(cp, c32.b, 4, 0);
            for (k=0; k<j; ++k)
            {
               out.push_back(c32.b[k]);
            }
         }
         return true;
         
      case UTF_16BE:
      case UTF_16LE:
         out.clear();
         n = UTF16Length(bytes);
         while (i < n)
         {
            cp = DecodeUTF16(bytes, n, &i, (e == UTF_16BE));
            if (!IsValidCodepoint(cp))
            {
               std::cerr << "Invalid code point " << cp << " (0x" << std::hex << cp << std::dec << ") in UTF-16 string" << std::endl;
               out.clear();
               return false;
            }
            j = EncodeUTF8(cp, c32.b, 4, 0);
            for (k=0; k<j; ++k)
            {
               out.push_back(c32.b[k]);
            }
         }
         return true;
      
      case UTF_32BE:
      case UTF_32LE:
      case UCS_4BE:
      case UCS_4LE:
         out.clear();
         n = UTF32Length(bytes);
         while (i < n)
         {
            cp = DecodeUTF32(bytes, n, &i, (e == UTF_32BE || e == UCS_4BE));
            if (!IsValidCodepoint(cp))
            {
               std::cerr << "Invalid code point " << cp << " (0x" << std::hex << cp << std::dec << ") in UTF-32 string" << std::endl;
               out.clear();
               return false;
            }
            j = EncodeUTF8(cp, c32.b, 4, 0);
            for (k=0; k<j; ++k)
            {
               out.push_back(c32.b[k]);
            }
         }
         return true;
      
      default:
         std::cout << "Not Yet Implemented" << std::endl;
         break;
      }
   }
   
   return false;
}

bool EncodeUTF8(const wchar_t *s, std::string &out)
{
   switch (sizeof(wchar_t))
   {
   case 4:
      return EncodeUTF8(IsBigEndian() ? UTF_32BE : UTF_32LE, (const char*)s, out);
   case 2:
      return EncodeUTF8(IsBigEndian() ? UTF_16BE : UTF_16LE, (const char*)s, out);
   default:
      return false;
   }
}

bool DecodeUTF8(const char *s, Encoding e, std::string &out)
{
   if (s)
   {
      size_t i = 0;
      size_t j = 0;
      char c8 = 0;
      Char16 c16;
      Char32 c32;
      const Byte *bytes = (const Byte*)s;
      size_t n = UTF8Length(bytes);
      unsigned int cp = InvalidCodepoint;
      
      switch (e)
      {
      case UTF_8:
         out = s;
         return true;
      
      case ASCII:
         out.clear();
         while (i < n)
         {
            cp = DecodeUTF8(bytes, n, &i);
            if (cp <= 127)
            {
               c8 = char(cp & 0xFF);
               out.push_back(c8);
            }
            else
            {
               return false;
            }
         }
         return true;
      
      case ASCII_ISO_8859_1:
         out.clear();
         while (i < n)
         {
            cp = DecodeUTF8(bytes, n, &i);
            if (cp <= 255)
            {
               out.push_back(char(cp & 0xFF));
            }
            else
            {
               return false;
            }
         }
         return true;
      
      case UCS_2BE:
      case UCS_2LE:
         out.clear();
         while (i < n)
         {
            c16.i = 0;
            if (EncodeUCS2(DecodeUTF8(bytes, n, &i), c16.b, 2, 0, e == UCS_2BE) == 2)
            {
               out.push_back(c16.b[0]);
               out.push_back(c16.b[1]);
            }
            else
            {
               return false;
            }
            
         }
         return true;
      
      case UTF_16BE:
      case UTF_16LE:
         out.clear();
         while (i < n)
         {
            c32.i = 0;
            j = EncodeUTF16(DecodeUTF8(bytes, n, &i), c32.b, 4, 0, e == UTF_16BE);
            if (j == 0)
            {
               return false;
            }
            else
            {
               out.push_back(c32.b[0]);
               out.push_back(c32.b[1]);
               if (j == 4)
               {
                  out.push_back(c32.b[2]);
                  out.push_back(c32.b[3]);
               }
            }
         }
         return true;
      
      case UCS_4BE:
      case UCS_4LE:
      case UTF_32BE:
      case UTF_32LE:
         out.clear();
         while (i < n)
         {
            c32.i = 0;
            if (EncodeUTF32(DecodeUTF8(bytes, n, &i), c32.b, 4, 0, (e == UCS_4BE || e == UTF_32BE)) == 4)
            {
               out.push_back(c32.b[0]);
               out.push_back(c32.b[1]);
               out.push_back(c32.b[2]);
               out.push_back(c32.b[3]);
            }
            else
            {
               return false;
            }
         }
         return true;
         
      default:
         break;
      }
   }
   
   return false;
}

bool DecodeUTF8(const char *s, std::wstring &out)
{
   std::string tmp;
   
   switch (sizeof(wchar_t))
   {
   case 4:
      if (!DecodeUTF8(s, IsBigEndian() ? UTF_32BE : UTF_32LE, tmp))
      {
         return false;
      }
      break;
   case 2:
      if (!DecodeUTF8(s, IsBigEndian() ? UTF_16BE : UTF_16LE, tmp))
      {
         return false;
      }
      break;
   default:
      return false;
   }
   
   out = (const wchar_t*) tmp.c_str();
   
   return true;
}

// ---

#include <gcore/config.h>
#include <gcore/platform.h>

std::ostream& PrintBytes(std::ostream &os, const void *ptr, size_t len, size_t spacing=2)
{
   os << "[" << len << "] 0x" << std::hex;
   const unsigned char *b = (const unsigned char *) ptr;
   for (size_t i=0, j=0; i<len; ++i, ++b)
   {
      if (*b == 0)
      {
         os << "00";
      }
      else
      {
         os << int(*b);
      }
      if (++j == spacing)
      {
         os << " ";
         j = 0;
      }
   }
   os << std::dec;
   return os;
}

#ifdef _WIN32

std::wstring ToWideString(const char *s, int codepage=-1)
{
   std::wstring wstr;
   
   if (s)
   {
      if (codepage == -1)
      {
         codepage = GetACP();
      }
      int slen = int(strlen(s));
      int wslen = MultiByteToWideChar(codepage, 0, s, slen, NULL, 0);
      wchar_t *ws = new wchar_t[wslen + 1];
      if (MultiByteToWideChar(codepage, 0, s, slen, ws, wslen + 1) != 0)
      {
         wstr = ws;
      }
   }
   
   return wstr;
}

std::string ToUTF8String(const wchar_t *ws)
{
   std::string str;
   
   if (ws)
   {
      int wslen = int(wcslen(ws));
      int slen = WideCharToMultiByte(CP_UTF8, 0, ws, wslen, NULL, 0, NULL, NULL);
      char *s = new char[slen + 1];
      if (WideCharToMultiByte(CP_UTF8, 0, ws, wslen, s, slen + 1, NULL, NULL) != 0)
      {
         str = s;
      }
   }
   
   return str;
}

void WindowsTests()
{
   const wchar_t *wtest = L"片道";
   size_t wlen = wcslen(wtest) * sizeof(wchar_t);
   const char *atest = "片道";
   size_t alen = strlen(atest);
   
   std::cout << "=== WINDOWS TESTS ===" << std::endl;
   
   std::string  nt0 = ToUTF8String(wtest);
   std::wstring wt0 = ToWideString(nt0.c_str(), CP_UTF8);
   
   std::wstring wt1 = ToWideString(atest);
   std::string  nt1 = ToUTF8String(wt1.c_str());
   
   std::wstring wt2 = ToWideString((const char*)wtest);
   std::string  nt2 = ToUTF8String(wt2.c_str());
   
   std::wstring wt3 = ToWideString(atest, CP_UTF8);
   std::string  nt3 = ToUTF8String(wt3.c_str());
   
   std::cout << "(wtest) "; PrintBytes(std::cout, wtest, wlen) << std::endl;
   std::cout << "(atest) "; PrintBytes(std::cout, atest, alen) << std::endl;
   std::cout << "(nt0)   "; PrintBytes(std::cout, nt0.c_str(), nt0.length()) << std::endl;
   std::cout << "(wt0)   "; PrintBytes(std::cout, wt0.c_str(), wt0.length() * sizeof(wchar_t)) << std::endl;
   std::cout << "(wt1)   "; PrintBytes(std::cout, wt1.c_str(), wt1.length() * sizeof(wchar_t)) << std::endl;
   std::cout << "(nt1)   "; PrintBytes(std::cout, nt1.c_str(), nt1.length()) << std::endl;
   std::cout << "(wt2)   "; PrintBytes(std::cout, wt2.c_str(), wt2.length() * sizeof(wchar_t)) << std::endl;
   std::cout << "(nt2)   "; PrintBytes(std::cout, nt2.c_str(), nt2.length()) << std::endl;
   std::cout << "(wt3)   "; PrintBytes(std::cout, wt3.c_str(), wt3.length() * sizeof(wchar_t)) << std::endl;
   std::cout << "(nt3)   "; PrintBytes(std::cout, nt3.c_str(), nt3.length()) << std::endl;
   
   std::wstring wt4;
   if (DecodeUTF8(atest, wt4))
   {
      std::cout << "(wt4)   "; PrintBytes(std::cout, wt4.c_str(), wt4.length() * sizeof(wchar_t)) << std::endl;
   }
   
   std::cout << "===" << std::endl;
   std::cout << std::endl;
}

#else

void WindowsTests()
{
}

#endif

typedef Codepoint (*DecodeFunc)(const Byte*, size_t, size_t*, bool);

int main(int, char **)
{
   std::cout << "sizeof(wchar_t)=" << sizeof(wchar_t) << std::endl;
   std::cout << "sizeof(unsigned int)=" << sizeof(unsigned int) << std::endl;
   std::cout << "IsBigEndian: " << IsBigEndian() << std::endl;
   std::cout << std::endl;


   // On linux/osx, seems that UTF-8 is the default encoding for narrow string
   // On windows, well... this is a mess that I'm still trying to decipher
   //   => compiling with _MBCS or _UNICODE does swap between 'A' and 'W' variants of win32 API functions
   //   => wchar_t is supported to be UTF-16 but... doesn't look like so...
   WindowsTests();


   const wchar_t *tests[6] = {
      L"片道",
      L"à",
      L"bien sûr.",
      L"Dans l’idéal évidement.",
      L"Si cela pouvait être implémenté proprement.",
      L"Plein de gros becs à mes petites chéries et aussi à la grande."
   };

   const Byte *bytes = (const Byte*) tests[0];
   size_t len = wcslen(tests[0]) * sizeof(wchar_t);

   PrintBytes(std::cout, tests[0], len) << std::endl;
   std::cout << std::hex << "0x" << tests[0][0] << " 0x" << tests[0][1] << std::dec << std::endl;
   std::cout << std::endl;
   
   DecodeFunc decode = (sizeof(wchar_t) == 4 ? (DecodeFunc)DecodeUTF32 : (DecodeFunc)DecodeUTF16);
   
   Codepoint cp[2];
   size_t j = 0;
   cp[0] = decode(bytes, len, &j, IsBigEndian());
   cp[1] = decode(bytes, len, &j, IsBigEndian());
   PrintBytes(std::cout, &cp[0], 2 * sizeof(Codepoint)) << std::endl;
   std::cout << std::hex << "0x" << cp[0] << " 0x" << cp[1] << std::dec << std::endl;
   std::cout << std::endl;
   
   Byte str[8];
   size_t n;
   
   for (int i=0; i<2; ++i)
   {
      std::cout << "Codepoint[" << i << "]: " << cp[i] << std::endl;
      
      memset(str, 0, 8);
      n = EncodeUTF32(cp[i], str, 8, 0, true);
      std::cout << "  => UTF32: [" << n << "] " << str << std::endl;
      std::cout << "  => Unicode: " << std::hex << DecodeUTF32(str, n, true) << std::dec << std::endl;
      std::cout << "  ";
      PrintBytes(std::cout, str, n) << std::endl;
      std::cout << std::endl;
      
      memset(str, 0, 8);
      n = EncodeUTF16(cp[i], str, 8, 0, true);
      std::cout << "  => UTF16: [" << n << "] " << str << std::endl;
      std::cout << "  => Unicode: " << std::hex << DecodeUTF16(str, n, true) << std::dec << std::endl;
      std::cout << "  ";
      PrintBytes(std::cout, str, n) << std::endl;
      std::cout << std::endl;
      
      memset(str, 0, 8);
      n = EncodeUTF8(cp[i], str, 8, 0);
      std::cout << "  => UTF8: [" << n << "] " << str << std::endl;
      std::cout << "  => Unicode: " << std::hex << DecodeUTF8(str, n) << std::dec << std::endl;
      std::cout << "  ";
      PrintBytes(std::cout, str, n) << std::endl;
      std::cout << std::endl;
   }
   
   std::string utf8;
   std::wstring wstr;
   std::string utf16;
   for (int i=0; i<6; ++i)
   {
      std::cout << "tests[" << i << "]" << std::endl;
      PrintBytes(std::cout, tests[i], wcslen(tests[i]) * sizeof(wchar_t), 4) << std::endl;
      if (EncodeUTF8(tests[i], utf8))
      {
         std::cout << tests[i] << " -> utf-8 -> [" << utf8.length() << "] \"" << utf8 << "\"" << std::endl;
         PrintBytes(std::cout, utf8.c_str(), utf8.length(), 1) << std::endl;
         
         if (!DecodeUTF8(utf8.c_str(), wstr) || wstr != tests[i])
         {
            std::cerr << "Re-encoding failed" << std::endl;
         }
         if (DecodeUTF8(utf8.c_str(), UTF_16, utf16))
         {
            std::cout << "-> utf-16 -> ";
            PrintBytes(std::cout, utf16.c_str(), utf16.length(), 2) << std::endl;
         }
      }
      std::cout << std::endl;
   }
   
   return 0;
}

