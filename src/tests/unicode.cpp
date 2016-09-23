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

union Char16
{
   char b[2];
   unsigned short i;
};

union Char32
{
   char b[4];
   unsigned int i;
};

// ---

static const char UTF8ContinuationCharBits = 0x80; // 10xx xxxx
static const char UTF8LeadingChar2Bits     = 0xC0; // 110x xxxx
static const char UTF8LeadingChar3Bits     = 0xE0; // 1110 xxxx
static const char UTF8LeadingChar4Bits     = 0xF0; // 1111 0xxx

static const char UTF8SingleCharMask       = 0x7F; // 0111 1111
static const char UTF8ContinuationCharMask = 0x3F; // 0011 1111
static const char UTF8LeadingChar2Mask     = 0x1F; // 0001 1111
static const char UTF8LeadingChar3Mask     = 0x0F; // 0000 1111
static const char UTF8LeadingChar4Mask     = 0x07; // 0000 0111

static const char UTF8ContinuationCharShift = 6;

unsigned int UTF8ByteRange[4][2] = {
   {0x00000000, 0x0000007F}, // 1 byte
   {0x00000080, 0x000007FF}, // 2 bytes
   {0x00000800, 0x0000FFFF}, // 3 bytes
   {0x00010000, 0x0010FFFF}  // 4 bytes
};

// ---

static const unsigned int UTF16HighSurrogateMask = 0x000FFC00;
static const unsigned int UTF16LowSurrogateMask  = 0x000003FF;

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
   Char16 c = {.i=0};
   c.b[0] = 0x00;
   c.b[1] = 0x01;
   return (c.i == 0x0001);
}

// Unicode code point are 32 bits but only 0x0000 0000 -> 0x0010 FFFF are used

const unsigned int InvalidCodepoint = 0xFFFFFFFF;

inline bool IsValidCodepoint(unsigned int cp)
{
   return (cp <= UTF8ByteRange[3][1]);// && (cp < 0xD800 || 0xDFFF < cp));
}

inline bool IsCombiningMark(unsigned int cp)
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

inline bool IsUTF8(const char *s)
{
   if (!s)
   {
      return false;
   }

   const unsigned char *bytes = (const unsigned char *)s;
   unsigned int cp = InvalidCodepoint;
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

      if ( (cp > 0x10FFFF) ||
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

// --- Encode functions ---

// -> size_t EncodeXXX(unsigned int cp, char *bytes, size_t len, size_t &pos)
inline size_t EncodeUTF8(unsigned int cp, char *bytes)
{
   if (!IsValidCodepoint(cp))
   {
      return 0;
   }
   else
   {
      if (cp <= UTF8ByteRange[0][1])
      {
         bytes[0] = char(cp & UTF8SingleCharMask);
         return 1;
      }
      else if (cp <= UTF8ByteRange[1][1])
      {
         bytes[1] = UTF8ContinuationCharBits | (cp & UTF8ContinuationCharMask);
         cp >>= UTF8ContinuationCharShift;
         bytes[0] = UTF8LeadingChar2Bits     | (cp & UTF8LeadingChar2Mask);
         return 2;
      }
      else if (cp <= UTF8ByteRange[2][1])
      {
         bytes[2] = UTF8ContinuationCharBits | (cp & UTF8ContinuationCharMask);
         cp >>= UTF8ContinuationCharShift;
         bytes[1] = UTF8ContinuationCharBits | (cp & UTF8ContinuationCharMask);
         cp >>= UTF8ContinuationCharShift;
         bytes[0] = UTF8LeadingChar3Bits     | (cp & UTF8LeadingChar3Mask);
         return 3;
      }
      else
      {
         bytes[3] = UTF8ContinuationCharBits | (cp & UTF8ContinuationCharMask);
         cp >>= UTF8ContinuationCharShift;
         bytes[2] = UTF8ContinuationCharBits | (cp & UTF8ContinuationCharMask);
         cp >>= UTF8ContinuationCharShift;
         bytes[1] = UTF8ContinuationCharBits | (cp & UTF8ContinuationCharMask);
         cp >>= UTF8ContinuationCharShift;
         bytes[0] = UTF8LeadingChar4Bits     | (cp & UTF8LeadingChar4Mask);
         return 4;
      }
   }
}

inline size_t EncodeUTF16(unsigned int cp, char *bytes, bool bigendian=true)
{
   const int (&order)[2] = UTF16BytesOrder[IsBigEndian() != bigendian];
   
   if (!IsValidCodepoint(cp))
   {
      return 0;
   }
   else
   {
      if (cp <= 0xD7FF)
      {
         bytes[order[0]] =  (cp & 0x00FF);
         bytes[order[1]] = ((cp & 0xFF00) >> 8);
         return 2;
      }
      else if (cp < 0xE000)
      {
         // Values are reserved for surrogate
         // => only write high surrogate?
         return 0;
      }
      else if (0xE000 <= cp && cp <= 0xFFFF)
      {
         bytes[order[0]] =  (cp & 0x00FF);
         bytes[order[1]] = ((cp & 0xFF00) >> 8);
         return 2;
      }
      else // 0x00010000 -> 0x0010FFFF
      {
         cp -= 0x00010000;
         unsigned short hs = 0xD800 + ((cp & UTF16HighSurrogateMask) >> 10);
         unsigned short ls = 0xDC00 + (cp & UTF16LowSurrogateMask);
         bytes[0 + order[0]] =  (hs & 0x00FF);
         bytes[0 + order[1]] = ((hs & 0xFF00) >> 8);
         bytes[2 + order[0]] =  (ls & 0x00FF);
         bytes[2 + order[1]] = ((ls & 0xFF00) >> 8);
         return 4;
      }
   }
}

inline size_t EncodeUCS2(unsigned int cp, char *bytes, bool bigendian=true)
{
   if (cp <= 0xD7FF || (0xE000 <= cp && cp <= 0xFFFF))
   {
      return EncodeUTF16(cp, bytes, bigendian);
   }
   else
   {
      return 0;
   }
}

inline size_t EncodeUTF32(unsigned int cp, char *bytes, bool bigendian=true)
{
   const int (&order)[4] = UTF32BytesOrder[IsBigEndian() != bigendian];
   
   bytes[order[0]] =  (cp & 0x000000FF);
   bytes[order[1]] = ((cp & 0x0000FF00) >> 8);
   bytes[order[2]] = ((cp & 0x00FF0000) >> 16);
   bytes[order[3]] = ((cp & 0xFF000000) >> 24);
   
   return 4;
}

// --- Decode functions ---

// -> unsigned int DecodeXXX(const char *bytes, size_t len, size_t &pos)
inline unsigned int DecodeUTF8(const char *bytes)
{
   unsigned int cp = 0;
   size_t n = 1;
   size_t i = 0;
   int shift = 0;
   
   while (i < n)
   {
      if (IsUTF8SingleChar(bytes[i]))
      {
         cp |= bytes[i];
      }
      else if (IsUTF8LeadingChar(bytes[i]))
      {
         if ((bytes[i] & ~UTF8LeadingChar2Mask) == UTF8LeadingChar2Bits)
         {
            shift = UTF8ContinuationCharShift;
            cp |= (bytes[i] & UTF8LeadingChar2Mask) << shift;
            n = 2;
         }
         else if ((bytes[i] & ~UTF8LeadingChar3Mask) == UTF8LeadingChar3Bits)
         {
            shift = 2 * UTF8ContinuationCharShift;
            cp |= (bytes[i] & UTF8LeadingChar3Mask) << shift;
            n = 3;
         }
         else if ((bytes[i] & ~UTF8LeadingChar4Mask) == UTF8LeadingChar4Bits)
         {
            shift = 3 * UTF8ContinuationCharShift;
            cp |= (bytes[i] & UTF8LeadingChar4Mask) << shift;
            n = 4;
         }
         else
         {
            return InvalidCodepoint;
         }
      }
      else
      {
         shift -= UTF8ContinuationCharShift;
         cp |= ((bytes[i] & UTF8ContinuationCharMask) << shift);
      }
      ++i;
   }
   
   // Check that cp actually requires n bytes to be encoded in UTF-8 (overlong)
   if (cp < UTF8ByteRange[n-1][0])
   {
      return InvalidCodepoint;
   }
   else
   {
      return cp;
   }
}

inline unsigned int DecodeUTF16(const char *bytes, bool bigendian=true)
{
   const int (&order)[2] = UTF16BytesOrder[IsBigEndian() != bigendian];
   
   Char16 c = {.i=0};
   
   c.b[order[0]] = bytes[0];
   c.b[order[1]] = bytes[1];
   
   if (0xD800 <= c.i && c.i <= 0xDFFF)
   {
      // surrogate 
      if (c.i >= 0xDC00)
      {
         // character should not start with low surrogate
         return InvalidCodepoint;
      }
      else
      {
         // Invalid for UCS-2
         // Need read next, if not a surrogate, invalud char
         Char16 hs = {.i=c.i};
         
         c.b[order[0]] = bytes[2];
         c.b[order[1]] = bytes[3];
         
         if (c.i < 0xDC00 || 0xDFFF < c.i)
         {
            // not a low surrogate
            return InvalidCodepoint;
         }
         else
         {
            return (((hs.i - 0xD800) << 10) | (c.i - 0xDC00));
         }
      }
   }
   else
   {
      return c.i;
   }
}

inline unsigned int DecodeUCS2(const char *bytes, bool bigendian=true)
{
   unsigned int cp = DecodeUTF16(bytes, bigendian);
   if (cp <= 0xD7FF || (0xE000 <= cp && cp <= 0xFFFF))
   {
      return cp;
   }
   else
   {
      return InvalidCodepoint;
   }
}

inline unsigned int DecodeUTF32(const char *bytes, bool bigendian=true)
{
   const int (&order)[4] = UTF32BytesOrder[IsBigEndian() != bigendian];
   
   Char32 c = {.i=0};
   
   c.b[order[0]] = bytes[0];
   c.b[order[1]] = bytes[1];
   c.b[order[2]] = bytes[2];
   c.b[order[3]] = bytes[3];
   
   return c.i;
}

// ---

inline bool EncodeUTF8(Encoding e, const char *s, std::string &out)
{
   if (s)
   {
      size_t i = 0;
      size_t j = 0;
      size_t k = 0;
      size_t n = 0;
      char c8 = 0;
      Char32 c32 = {.i=0};
      const char *cc = s;
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
               j = EncodeUTF8(cp & 0x00FF, c32.b);
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
         for (cc=s; cc[0]!=0 || cc[1]!=0; cc+=2)
         {
            cp = DecodeUCS2(cc, (e == UCS_2BE));
            if (!IsValidCodepoint(cp))
            {
               std::cerr << "Invalid code point " << cp << " in UCS-2 string" << std::endl;
               out.clear();
               return false;
            }
            j = EncodeUTF8(cp, c32.b);
            for (k=0; k<j; ++k)
            {
               out.push_back(c32.b[k]);
            }
         }
         return true;
         
      case UTF_16BE:
      case UTF_16LE:
         out.clear();
         for (cc=s; cc[0]!=0 || cc[1]!=0; cc+=2)
         {
            cp = DecodeUTF16(cc, (e == UTF_16BE));
            if (!IsValidCodepoint(cp))
            {
               std::cerr << "Invalid code point " << cp << " in UTF-16 string" << std::endl;
               out.clear();
               return false;
            }
            else if (cp >= 0x10000)
            {
               // surrogate pair, encoded as 4 bytes
               cc += 2;
            }
            j = EncodeUTF8(cp, c32.b);
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
         for (cc=s; cc[0]!= 0 || cc[1]!=0 || cc[2]!=0 || cc[3]!=0; cc+=4)
         {
            cp = DecodeUTF32(cc, (e == UTF_32BE || e == UCS_4BE));
            if (!IsValidCodepoint(cp))
            {
               std::cerr << "Invalid code point " << cp << " in UTF-32 string" << std::endl;
               out.clear();
               return false;
            }
            j = EncodeUTF8(cp, c32.b);
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

inline bool EncodeUTF8(const wchar_t *s, std::string &out)
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

inline bool DecodeUTF8(const char *s, Encoding e, std::string &out)
{
   if (s)
   {
      size_t n = 0;
      char c8 = 0;
      Char16 c16 = {.i=0};
      Char32 c32 = {.i=0};
      const char *cc = s;
      unsigned int cp = InvalidCodepoint;
      
      switch (e)
      {
      case UTF_8:
         out = s;
         return true;
      
      case ASCII:
         for (cc=s; *cc!=0; ++cc)
         {
            cp = DecodeUTF8(cc);
            if (cp == InvalidCodepoint || cp > 127)
            {
               return false;
            }
            c8 = char(cp & 0xFF);
            out.push_back(c8);
         }
         return true;
      
      case ASCII_ISO_8859_1:
         for (cc=s; *cc!=0; ++cc)
         {
            cp = DecodeUTF8(cc);
            if (cp == InvalidCodepoint || cp > 255)
            {
               return false;
            }
            out.push_back(char(cp & 0xFF));
         }
         return true;
      
      case UCS_2BE:
      case UCS_2LE:
         for (cc=s; *cc!=0; ++cc)
         {
            if (EncodeUCS2(DecodeUTF8(cc), c16.b, e == UCS_2BE) == 0)
            {
               return false;
            }
            out.push_back(c16.b[0]);
            out.push_back(c16.b[1]);
         }
         return true;
      
      case UTF_16BE:
      case UTF_16LE:
         for (cc=s; *cc!=0; ++cc)
         {
            n = EncodeUTF16(DecodeUTF8(cc), c32.b, e == UTF_16BE);
            if (n == 0)
            {
               return false;
            }
            out.push_back(c32.b[0]);
            out.push_back(c32.b[1]);
            if (n == 4)
            {
               out.push_back(c32.b[2]);
               out.push_back(c32.b[3]);
            }
         }
         return true;
      
      case UCS_4BE:
      case UCS_4LE:
      case UTF_32BE:
      case UTF_32LE:
         for (cc=s; *cc!=0; ++cc)
         {
            n = EncodeUTF32(DecodeUTF8(cc), c32.b, (e == UCS_4BE || e == UTF_32BE));
            if (n == 0)
            {
               return false;
            }
            out.push_back(c32.b[0]);
            out.push_back(c32.b[1]);
            out.push_back(c32.b[2]);
            out.push_back(c32.b[3]);
         }
         return true;
         
      default:
         break;
      }
   }
   
   return false;
}

inline bool DecodeUTF8(const char *s, std::wstring &out)
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

inline std::ostream& PrintBytes(std::ostream &os, const void *ptr, size_t len, size_t spacing=2)
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

int main(int, char **)
{
   const wchar_t *tests[6] = {
      L"片道",
      L"à",
      L"bien sûr.",
      L"Dans l’idéal évidement.",
      L"Si cela pouvait être implémenté proprement.",
      L"Plein de gros becs à mes petites chéries et aussi à la grande."
   };
   // On linux, seems that UTF-8 is the default encoding for narrow string
   const char *results[6] = {
      "片道",
      "à",
      "bien sûr.",
      "Dans l’idéal évidement.",
      "Si cela pouvait être implémenté proprement.",
      "Plein de gros becs à mes petites chéries et aussi à la grande."
   };
   
   size_t len = wcslen(tests[0]);
   
   std::cout << "sizeof(wchar_t)=" << sizeof(wchar_t) << std::endl;
   std::cout << "sizeof(unsigned int)=" << sizeof(unsigned int) << std::endl;
   std::cout << "IsBigEndian: " << IsBigEndian() << std::endl;
   std::cout << std::endl;
   
   PrintBytes(std::cout, tests[0], len * sizeof(wchar_t)) << std::endl;
   std::cout << std::hex << "0x" << tests[0][0] << " 0x" << tests[0][1] << std::dec << std::endl;
   std::cout << std::endl;
   
   unsigned int cp[2];
   cp[0] = DecodeUTF32((const char*)tests[0], IsBigEndian());
   cp[1] = DecodeUTF32((const char*)(tests[0] + 1), IsBigEndian());
   PrintBytes(std::cout, &cp[0], 2 * sizeof(unsigned int)) << std::endl;
   std::cout << std::hex << "0x" << cp[0] << " 0x" << cp[1] << std::dec << std::endl;
   std::cout << std::endl;
   
   char str[8];
   size_t n;
   
   for (int i=0; i<2; ++i)
   {
      std::cout << "Codepoint[" << i << "]: " << cp[i] << std::endl;
      
      memset(str, 0, 8);
      n = EncodeUTF32(cp[i], str, true);
      std::cout << "  => UTF32: [" << n << "] " << str << std::endl;
      std::cout << "  => Unicode: " << std::hex << DecodeUTF32(str, true) << std::dec << std::endl;
      std::cout << "  ";
      PrintBytes(std::cout, str, n) << std::endl;
      std::cout << std::endl;
      
      memset(str, 0, 8);
      n = EncodeUTF16(cp[i], str, true);
      std::cout << "  => UTF16: [" << n << "] " << str << std::endl;
      std::cout << "  => Unicode: " << std::hex << DecodeUTF16(str, true) << std::dec << std::endl;
      std::cout << "  ";
      PrintBytes(std::cout, str, n) << std::endl;
      std::cout << std::endl;
      
      memset(str, 0, 8);
      n = EncodeUTF8(cp[i], str);
      std::cout << "  => UTF8: [" << n << "] " << str << std::endl;
      std::cout << "  => Unicode: " << std::hex << DecodeUTF8(str) << std::dec << std::endl;
      std::cout << "  ";
      PrintBytes(std::cout, str, n) << std::endl;
      std::cout << std::endl;
   }
   
   std::string utf8;
   for (int i=0; i<6; ++i)
   {
      std::cout << "tests[" << i << "]" << std::endl;
      PrintBytes(std::cout, tests[i], wcslen(tests[i]) * sizeof(wchar_t), 4) << std::endl;
      if (EncodeUTF8(tests[i], utf8))
      {
         std::cout << tests[i] << " -> utf-8 -> [" << utf8.length() << "] \"" << utf8 << "\"" << std::endl;
         PrintBytes(std::cout, utf8.c_str(), utf8.length(), 1) << std::endl;
         if (utf8 != results[i])
         {
            std::cout << "Doesn't match \"" << results[i] << "\"" << std::endl;
         }
      }
      std::cout << std::endl;
   }
   
   return 0;
}

