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

#include <gcore/encoding.h>
#include <gcore/hashmap.h>
#ifdef _WIN32
#include <gcore/platform.h>
#endif

#define C0_NUL 0x0000 // (^@) [\0] null
#define C0_SOH 0x0001 // (^A)      start of heading
#define C0_STX 0x0002 // (^B)      start of text
#define C0_ETX 0x0003 // (^C)      end of text
#define C0_EOT 0x0004 // (^D)      end of transmission
#define C0_ENQ 0x0005 // (^E)      enquiry
#define C0_ACK 0x0006 // (^F)      acknowledge
#define C0_BEL 0x0007 // (^G) [\a] bell
#define C0_BS  0x0008 // (^H) [\b] backspace
#define C0_HT  0x0009 // (^I) [\t] horizontal tabulation
#define C0_LF  0x000A // (^J) [\n] line feed
#define C0_VT  0x000B // (^K) [\v] vertical tabulation
#define C0_FF  0x000C // (^L) [\f] form feed
#define C0_CR  0x000D // (^M) [\r] carriage return
#define C0_SO  0x000E // (^N)      shift out
#define C0_SI  0x000F // (^O)      shift out
#define C0_DLE 0x0010 // (^P)      data link escape
#define C0_DC1 0x0011 // (^Q)      device control 1 (XON)
#define C0_DC2 0x0012 // (^R)      device control 2
#define C0_DC3 0x0013 // (^S)      device control 3 (XOFF)
#define C0_DC4 0x0014 // (^T)      device control 4
#define C0_NAK 0x0015 // (^U)      negative acknowledge
#define C0_SYN 0x0016 // (^V)      synchronous idle
#define C0_ETB 0x0017 // (^W)      end of transmission block
#define C0_CAN 0x0018 // (^X)      cancel
#define C0_EM  0x0019 // (^Y)      end of medium
#define C0_SUB 0x001A // (^Z)      substitute
#define C0_ESC 0x001B // (^[) [\e] escape
#define C0_FS  0x001C // (^\)      file separator
#define C0_GS  0x001D // (^])      group separator
#define C0_RS  0x001E // (^^)      record separator
#define C0_US  0x001F // (^_)      unit separator
#define C0_SP  0x0020 // (  )      space
#define C0_DEL 0x007F // (^?)      backspace

#define C0_Control_Codes \
   C0_NUL, C0_SOH, C0_STX, C0_ETX, C0_EOT, C0_ENQ, C0_ACK, C0_BEL, C0_BS,  C0_HT, C0_LF,  C0_VT,  C0_FF, C0_CR, C0_SO, C0_SI, \
   C0_DLE, C0_DC1, C0_DC2, C0_DC3, C0_DC4, C0_NAK, C0_SYN, C0_ETB, C0_CAN, C0_EM, C0_SUB, C0_ESC, C0_FS, C0_GS, C0_RS, C0_US

#define C1_PAD  0x0080 // (ESC @) padding character
#define C1_HOP  0x0081 // (ESC A) high octet preset 
#define C1_BPH  0x0082 // (ESC B) break permitted here
#define C1_NBH  0x0083 // (ESC C) no break here 
#define C1_IND  0x0084 // (ESC D) index
#define C1_NEL  0x0085 // (ESC E) next line
#define C1_SSA  0x0086 // (ESC F) start of selected area
#define C1_ESA  0x0087 // (ESC G) end of selected area
#define C1_HTS  0x0088 // (ESC H) horizontal tabulation set 
#define C1_HTJ  0x0089 // (ESC I) horizontal tabulation with justification 
#define C1_VTS  0x008A // (ESC J) vertical tabulation set 
#define C1_PLD  0x008B // (ESC K) partial line down
#define C1_PLU  0x008C // (ESC L) partial line up 
#define C1_RI   0x008D // (ESC M) reverse line feed 
#define C1_SS2  0x008E // (ESC N) single shift 2
#define C1_SS3  0x008F // (ESC O) single shift 3
#define C1_DCS  0x0090 // (ESC P) device control string
#define C1_PU1  0x0091 // (ESC Q) private use 1
#define C1_PU2  0x0092 // (ESC R) private use 2
#define C1_STS  0x0093 // (ESC S) set transmit state
#define C1_CCH  0x0094 // (ESC T) cancel character
#define C1_MW   0x0095 // (ESC U) message waiting
#define C1_SPA  0x0096 // (ESC V) start of protected area
#define C1_EPA  0x0097 // (ESC W) end of protected area
#define C1_SOS  0x0098 // (ESC X) start of string
#define C1_SGCI 0x0099 // (ESC Y) single graphic character introducer
#define C1_SCI  0x009A // (ESC Z) single character introducer
#define C1_CSI  0x009B // (ESC [) control sequence introducer
#define C1_ST   0x009C // (ESC \) string terminator
#define C1_OSC  0x009D // (ESC ]) operating system command
#define C1_PM   0x009E // (ESC ^) privacy message
#define C1_APC  0x009F // (ESC _) application program command

#define C1_Control_Codes \
   C1_PAD, C1_HOP, C1_BPH, C1_NBH, C1_IND, C1_NEL, C1_SSA, C1_ESA, C1_HTS, C1_HTJ, C1_VTS, C1_PLD, C1_PLU, C1_RI, C1_SS2, C1_SS3, \
   C1_DCS, C1_PU1, C1_PU2, C1_STS, C1_CCH, C1_MW, C1_SPA, C1_EPA, C1_SOS, C1_SGCI, C1_SCI, C1_CSI, C1_ST, C1_OSC, C1_PM, C1_APC

#define C_REPL 0xFFFD

#define ASCII_Codepoints \
   C0_Control_Codes, \
   C0_SP, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, \
   0x30,  0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, \
   0x40,  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, \
   0x50,  0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, \
   0x60,  0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, \
   0x70,  0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, C0_DEL


namespace gcore
{

// --- Private types

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

// --- Private constants

// Unicode code point are 32 bits but only 0x0000 0000 -> 0x0010 FFFF are used
const Codepoint InvalidCodepoint = 0xFFFFFFFF;

// UTF-8
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

// UTF-16
static const Byte4 UTF16HighSurrogateMask = 0x000FFC00;
static const Byte4 UTF16LowSurrogateMask  = 0x000003FF;

static const int UTF16BytesOrder[2][2] = {
   {0, 1},
   {1, 0} // swap
};

// UTF-32
static const int UTF32BytesOrder[2][4] = {
   {0, 1, 2, 3},
   {3, 2, 1, 0} // swap
};

// ISO 8859 1~16 codepoints
const Codepoint iso_8859[15][256] =
{
   // ISO 8859-1
   {
      ASCII_Codepoints,
      C1_Control_Codes,
      0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
      0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
      0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
      0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
      0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
      0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
   },
   // ISO 8859-2
   {
      ASCII_Codepoints,
      C1_Control_Codes,
      0x00A0, 0x0104, 0x02D8, 0x0141, 0x00A4, 0x013D, 0x015A, 0x00A7, 0x00A8, 0x0160, 0x015E, 0x0164, 0x0179, 0x00AD, 0x017D, 0x017B,
      0x00B0, 0x0105, 0x02DB, 0x0142, 0x00B4, 0x013E, 0x015B, 0x02C7, 0x00B8, 0x0161, 0x015F, 0x0165, 0x017A, 0x02DD, 0x017E, 0x017C,
      0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7, 0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE, 0x010E,
      0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7, 0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF,
      0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7, 0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F,
      0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7, 0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9
   },
   // ISO 8859-3
   {
      ASCII_Codepoints,
      C1_Control_Codes,
      0x00A0, 0x0126, 0x02D8, 0x00A3, 0x00A4, 0x00A5, 0x0124, 0x00A7, 0x00A8, 0x0130, 0x015E, 0x011E, 0x0134, 0x00AD, 0x00AE, 0x017B,
      0x00B0, 0x0127, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x0125, 0x00B7, 0x00B8, 0x0131, 0x015F, 0x011F, 0x0135, 0x00BD, 0x00BE, 0x017C,
      0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x010A, 0x0108, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
      0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x0120, 0x00D6, 0x00D7, 0x011C, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x016C, 0x015C, 0x00DF,
      0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x010B, 0x0109, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
      0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x0121, 0x00F6, 0x00F7, 0x011D, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x016D, 0x015D, 0x02D9
   },
   // ISO 8859-4
   {
      ASCII_Codepoints,
      C1_Control_Codes,
      0x00A0, 0x0104, 0x0138, 0x0156, 0x00A4, 0x0128, 0x013B, 0x00A7, 0x00A8, 0x0160, 0x0112, 0x0122, 0x0166, 0x00AD, 0x017D, 0x00AF,
      0x00B0, 0x0105, 0x02DB, 0x0157, 0x00B4, 0x0129, 0x013C, 0x02C7, 0x00B8, 0x0161, 0x0113, 0x0123, 0x0167, 0x014A, 0x017E, 0x014B,
      0x0100, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x012E, 0x010C, 0x00C9, 0x0118, 0x00CB, 0x0116, 0x00CD, 0x00CE, 0x012A,
      0x0110, 0x0145, 0x014C, 0x0136, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x0172, 0x00DA, 0x00DB, 0x00DC, 0x0168, 0x016A, 0x00DF,
      0x0101, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x012F, 0x010E, 0x00E9, 0x0119, 0x00EB, 0x0117, 0x00ED, 0x00EE, 0x012B,
      0x0111, 0x0146, 0x014D, 0x0137, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x0173, 0x00FA, 0x00FB, 0x00FC, 0x0169, 0x016B, 0x02D9
   },
   // ISO 8859-5
   {
      ASCII_Codepoints,
      C1_Control_Codes,
      0x00A0, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407, 0x0408, 0x0409, 0x040A, 0x040B, 0x040C, 0x00AD, 0x040E, 0x040F,
      0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
      0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
      0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
      0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F,
      0x2116, 0x0451, 0x0452, 0x0453, 0x0454, 0x0455, 0x0456, 0x0457, 0x0458, 0x0459, 0x045A, 0x045B, 0x045C, 0x00A7, 0x045E, 0x045F
   },
   // ISO 8859-6
   {
      ASCII_Codepoints,
      C1_Control_Codes,
      0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x060C, 0x00AD, 0x00AE, 0x00AF,
      0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x061B, 0x00BC, 0x00BD, 0x00BE, 0x061F,
      0x00C0, 0x0621, 0x0622, 0x0623, 0x0624, 0x0625, 0x0626, 0x0627, 0x0628, 0x0629, 0x062A, 0x062B, 0x062C, 0x062D, 0x062E, 0x062F,
      0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635, 0x0636, 0x0637, 0x0638, 0x0639, 0x063A, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
      0x0640, 0x0641, 0x0642, 0x0643, 0x0644, 0x0645, 0x0646, 0x0647, 0x0648, 0x0649, 0x064A, 0x064B, 0x064C, 0x064D, 0x064E, 0x064F,
      0x0650, 0x0651, 0x0652, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
   },
   // ISO 8859-7
   {
      ASCII_Codepoints,
      C1_Control_Codes,
      0x00A0, 0x2018, 0x2019, 0x00A3, 0x20AC, 0x20AF, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x037A, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x2015,
      0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x0384, 0x0385, 0x0386, 0x00B7, 0x0388, 0x0389, 0x038A, 0x00BB, 0x038C, 0x00BD, 0x038E, 0x038F,
      0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397, 0x0398, 0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F,
      0x03A0, 0x03A1, 0x00D2, 0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7, 0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x03AC, 0x03AD, 0x03AE, 0x03AF,
      0x03B0, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6, 0x03B7, 0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF,
      0x03C0, 0x03C1, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7, 0x03C8, 0x03C9, 0x03CA, 0x03CB, 0x03CC, 0x03CD, 0x03CE, 0x00FF
   },
   // ISO 8859-8
   {
      ASCII_Codepoints,
      C1_Control_Codes,
      0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00D7, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
      0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00F7, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
      0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
      0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, C_REPL, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x2017,
      0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7, 0x05D8, 0x05D9, 0x05DA, 0x05DB, 0x05DC, 0x05DD, 0x05DE, 0x05DF,
      0x05E0, 0x05E1, 0x05E2, 0x05E3, 0x05E4, 0x05E5, 0x05E6, 0x05E7, 0x05E8, 0x05E9, 0x05EA, 0x00FB, 0x00FC, 0x200E, 0x200F, 0x00FF
   },
   // ISO 8859-9
   {
      ASCII_Codepoints,
      C1_Control_Codes,
      0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
      0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
      0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
      0x011E, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x0130, 0x015E, 0x00DF,
      0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
      0x011F, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x0131, 0x015F, 0x00FF
   },
   // ISO 8859-10
   {
      ASCII_Codepoints,
      C1_Control_Codes,
      0x00A0, 0x0104, 0x0112, 0x0122, 0x012A, 0x0128, 0x0136, 0x00A7, 0x013B, 0x0110, 0x0160, 0x0166, 0x017D, 0x00AD, 0x016A, 0x014A,
      0x00B0, 0x0105, 0x0113, 0x0123, 0x012B, 0x0129, 0x0137, 0x00B7, 0x013C, 0x0111, 0x0161, 0x0167, 0x017E, 0x2015, 0x016B, 0x014B,
      0x0100, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x012E, 0x010C, 0x00C9, 0x0118, 0x00CB, 0x0116, 0x00CD, 0x00CE, 0x00CF,
      0x00D0, 0x0145, 0x014C, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x0168, 0x00D8, 0x0172, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
      0x0101, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x012F, 0x010D, 0x00E9, 0x0119, 0x00EB, 0x0117, 0x00ED, 0x00EE, 0x00EF,
      0x00F0, 0x0146, 0x014D, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x0169, 0x00F8, 0x0173, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x0138
   },
   // ISO 8859-11
   {
      ASCII_Codepoints,
      C1_Control_Codes,
      0x00A0, 0x0E01, 0x0E02, 0x0E03, 0x0E04, 0x0E05, 0x0E06, 0x0E07, 0x0E08, 0x0E09, 0x0E0A, 0x0E0B, 0x0E0C, 0x0E0D, 0x0E0E, 0x0E0F,
      0x0E10, 0x0E11, 0x0E12, 0x0E13, 0x0E14, 0x0E15, 0x0E16, 0x0E17, 0x0E18, 0x0E19, 0x0E1A, 0x0E1B, 0x0E1C, 0x0E1D, 0x0E1E, 0x0E1F,
      0x0E20, 0x0E21, 0x0E22, 0x0E23, 0x0E24, 0x0E25, 0x0E26, 0x0E27, 0x0E28, 0x0E29, 0x0E2A, 0x0E2B, 0x0E2C, 0x0E2D, 0x0E2E, 0x0E2F,
      0x0E30, 0x0E31, 0x0E32, 0x0E33, 0x0E34, 0x0E35, 0x0E36, 0x0E37, 0x0E38, 0x0E39, 0x0E3A, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x0E3F,
      0x0E40, 0x0E41, 0x0E42, 0x0E43, 0x0E44, 0x0E45, 0x0E46, 0x0E47, 0x0E48, 0x0E49, 0x0E4A, 0x0E4B, 0x0E4C, 0x0E4D, 0x0E4E, 0x0E4F,
      0x0E50, 0x0E51, 0x0E52, 0x0E53, 0x0E54, 0x0E55, 0x0E56, 0x0E57, 0x0E58, 0x0E59, 0x0E5A, 0x0E5B, 0x00FC, 0x00FD, 0x00FE, 0x00FF
   },
   // ISO 8859-13
   {
      ASCII_Codepoints,
      C1_Control_Codes,
      0x00A0, 0x201D, 0x00A2, 0x00A3, 0x00A4, 0x201E, 0x00A6, 0x00A7, 0x00D8, 0x00A9, 0x0156, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00C6,
      0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x201C, 0x00B5, 0x00B6, 0x00B7, 0x00F8, 0x00B9, 0x0157, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00E6,
      0x0104, 0x012E, 0x0100, 0x0106, 0x00C4, 0x00C5, 0x0118, 0x0112, 0x010C, 0x00C9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012A, 0x013B,
      0x0160, 0x0143, 0x0145, 0x00D3, 0x014C, 0x00D5, 0x00D6, 0x00D7, 0x0172, 0x0141, 0x015A, 0x016A, 0x00DC, 0x017B, 0x017D, 0x00DF,
      0x0105, 0x012F, 0x0101, 0x0197, 0x00E4, 0x00E5, 0x0119, 0x0113, 0x010D, 0x00E9, 0x017A, 0x0117, 0x0123, 0x0137, 0x012B, 0x013C,
      0x0161, 0x0144, 0x0146, 0x00F3, 0x014D, 0x00F5, 0x00F6, 0x00F7, 0x0173, 0x0142, 0x015B, 0x016B, 0x00FC, 0x017C, 0x017E, 0x2019
   },
   // ISO 8859-14
   {
      ASCII_Codepoints,
      C1_Control_Codes,
      0x00A0, 0x1E02, 0x1E03, 0x00A3, 0x010A, 0x010B, 0x1E0A, 0x00A7, 0x1E80, 0x00A9, 0x1E82, 0x1E0B, 0x1EF2, 0x00AD, 0x00AE, 0x0178,
      0x1E1E, 0x1E1F, 0x0120, 0x0121, 0x1E40, 0x1E41, 0x00B6, 0x1E56, 0x1E81, 0x1E57, 0x1E83, 0x1E60, 0x1EF3, 0x1E84, 0x1E85, 0x1E61,
      0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
      0x0174, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x1E6A, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x0176, 0x00DF,
      0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
      0x0175, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x1E6B, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x0177, 0x00FF
   },
   // ISO 8859-15
   {
      ASCII_Codepoints,
      C1_Control_Codes,
      0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x20AC, 0x00A5, 0x0160, 0x00A7, 0x0161, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
      0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x017D, 0x00B5, 0x00B6, 0x00B7, 0x017E, 0x00B9, 0x00BA, 0x00BB, 0x0152, 0x0153, 0x0178, 0x00BF,
      0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
      0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
      0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
      0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
   },
   // ISO 8859-16
   {
      ASCII_Codepoints,
      C1_Control_Codes,
      0x00A0, 0x0104, 0x0105, 0x0141, 0x20AC, 0x201E, 0x0160, 0x00A7, 0x0161, 0x00A9, 0x0218, 0x00AB, 0x0179, 0x00AD, 0x017A, 0x017B,
      0x00B0, 0x00B1, 0x010C, 0x0142, 0x017D, 0x201D, 0x00B6, 0x00B7, 0x017E, 0x010D, 0x0219, 0x00BB, 0x0152, 0x0153, 0x0178, 0x017C,
      0x00C0, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0106, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
      0x0110, 0x0143, 0x00D2, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x015A, 0x0170, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x0118, 0x021A, 0x00DF,
      0x00E0, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x0107, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
      0x0111, 0x0144, 0x00F2, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x015B, 0x0171, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x0119, 0x021B, 0x00FF
   }
};

// Reverse mapping for ISO 8859 1~16
HashMap<Codepoint, unsigned char> iso_8859_rev[15];

// --- Private functions

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

static void InitASCIIReverseMappings()
{
   if (iso_8859_rev[0].size() > 0)
   {
      return;
   }
   
   for (int i=0; i<15; ++i)
   {
      const Codepoint (&mapping)[256] = iso_8859[i];
      HashMap<Codepoint, unsigned char> &rmapping = iso_8859_rev[i];
      
      for (size_t j=0; j<256; ++j)
      {
#ifdef _DEBUG
         if (rmapping.find(mapping[j]) != rmapping.end())
         {
            std::cerr << "Duplicate code point in '" << EncodingString(ASCII_ISO_8859_1 + i) << "': " << std::hex << mapping[j] << std::dec << std::endl;
         }
#endif
         rmapping[mapping[j]] = (unsigned char)(j & 0x00FF);
      }
   }
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

// Length functions

static size_t UTF8Length(const Byte *bytes)
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

static size_t UTF16Length(const Byte *bytes)
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

static size_t UCS2Length(const Byte *bytes)
{
   return UTF16Length(bytes);
}

static size_t UTF32Length(const Byte *bytes)
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

// Single codepoint encode functions

static size_t EncodeUTF8(Codepoint cp, Byte *bytes, size_t maxlen, size_t pos)
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

static size_t EncodeUTF16(Codepoint cp, Byte *bytes, size_t maxlen, size_t pos, bool bigendian)
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

static size_t EncodeUCS2(Codepoint cp, Byte *bytes, size_t maxlen, size_t pos, bool bigendian)
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

static size_t EncodeUTF32(Codepoint cp, Byte *bytes, size_t maxlen, size_t pos, bool bigendian)
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

// Single codepoint decode functions

static Codepoint DecodeUTF8(const Byte *bytes, size_t len, size_t *_pos)
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

static Codepoint DecodeUTF16(const Byte *bytes, size_t len, size_t *_pos, bool bigendian)
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

static Codepoint DecodeUCS2(const Byte *bytes, size_t len, size_t *pos, bool bigendian)
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

static Codepoint DecodeUTF32(const Byte *bytes, size_t len, size_t *_pos, bool bigendian)
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

inline Codepoint DecodeUTF8(const Byte *bytes, size_t len)
{
   size_t pos = 0;
   return DecodeUTF8(bytes, len, &pos);
}

inline Codepoint DecodeUTF16(const Byte *bytes, size_t len, bool bigendian)
{
   size_t pos = 0;
   return DecodeUTF16(bytes, len, &pos, bigendian);
}

inline Codepoint DecodeUCS2(const Byte *bytes, size_t len, bool bigendian)
{
   size_t pos = 0;
   return DecodeUCS2(bytes, len, &pos, bigendian);
}

inline Codepoint DecodeUTF32(const Byte *bytes, size_t len, bool bigendian)
{
   size_t pos = 0;
   return DecodeUTF32(bytes, len, &pos, bigendian);
}

// --- Public functions

const char* EncodingString(Encoding e)
{
   const char* sEncodingStr[MAX_ENCODING] =
   {
      "ascii",
      "iso-8859-1",
      "iso-8859-2",
      "iso-8859-3",
      "iso-8859-4",
      "iso-8859-5",
      "iso-8859-6",
      "iso-8859-7",
      "iso-8859-8",
      "iso-8859-9",
      "iso-8859-10",
      "iso-8859-11",
      "iso-8859-13",
      "iso-8859-14",
      "iso-8859-15",
      "iso-8859-16",
      "utf-8",
      "ucs-2",
      "ucs-2le",
      "utf-16",
      "utf-16le",
      "ucs-4",
      "ucs-4le",
      "utf-32",
      "utf-32le"
   };
   
   int idx = int(e);
   return ((e < 0 || e >= MAX_ENCODING) ? 0 : sEncodingStr[idx]);
}

bool IsBigEndian()
{
   // If the machine the code was compiled on is big endian
   // 0x01 will be interpreted as least significant byte in c.i
   Char16 c; // = {.i=0};
   c.b[0] = 0x00;
   c.b[1] = 0x01;
   return (c.i == 0x0001);
}

bool IsASCII(const char *s)
{
   if (!s)
   {
      return false;
   }
   
   const Byte *bytes = (const Byte*)s;
   
   while (*bytes != 0x00)
   {
      if (*bytes > 127)
      {
         return false;
      }
      ++bytes;
   }
   
   return true;
}

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

bool EncodeUTF8(Encoding e, const char *s, std::string &out)
{
   if (s)
   {
      size_t len = 0;
      
      switch (e)
      {
      case UCS_2BE:
      case UCS_2LE:
         len = UCS2Length((const Byte*)s);
         break;
         
      case UTF_16BE:
      case UTF_16LE:
         len = UTF16Length((const Byte*)s);
         break;
      
      case UTF_32BE:
      case UTF_32LE:
      case UCS_4BE:
      case UCS_4LE:
         len = UTF32Length((const Byte*)s);
         break;
      
      case UTF_8:
         out = s;
         return true;
      
      default:
         len = strlen(s);
      }
      
      return EncodeUTF8(e, s, len, out);
   }
   else
   {
      return false;
   }
}

bool EncodeUTF8(Encoding e, const char *s, size_t len, std::string &out)
{
   if (s)
   {
      size_t i = 0;
      size_t j = 0;
      size_t k = 0;
      size_t n = len;
      char c8 = 0;
      Char32 c32;
      const Byte *bytes = (const Byte*)s;
      unsigned int cp = InvalidCodepoint;
      
      switch (e)
      {
      case UTF_8:
         //out = s;
         n = std::min(n, UTF8Length(bytes));
         out.assign(s, n);
         return true;

      case ASCII:
         //n = strlen(s);
         n = std::min(n, strlen(s));
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
      case ASCII_ISO_8859_2:
      case ASCII_ISO_8859_3:
      case ASCII_ISO_8859_4:
      case ASCII_ISO_8859_5:
      case ASCII_ISO_8859_6:
      case ASCII_ISO_8859_7:
      case ASCII_ISO_8859_8:
      case ASCII_ISO_8859_9:
      case ASCII_ISO_8859_10:
      case ASCII_ISO_8859_11:
      case ASCII_ISO_8859_13:
      case ASCII_ISO_8859_14:
      case ASCII_ISO_8859_15:
      case ASCII_ISO_8859_16:
         {
            const Codepoint (&mapping)[256] = iso_8859[e - ASCII_ISO_8859_1];
            
            //n = strlen(s);
            n = std::min(n, strlen(s));
            out.reserve(n * 4);
            out.clear();
            for (i=0; i<n; ++i)
            {
               c8 = s[i];
               cp = mapping[int(c8) & 0x00FF];
               j = EncodeUTF8(cp, c32.b, 4, 0);
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
         }
         return true;
      
      case UCS_2BE:
      case UCS_2LE:
         out.reserve(n * 4);
         out.clear();
         //n = UCS2Length(bytes);
         n = std::min(n, UCS2Length(bytes));
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
         out.reserve(n * 4);
         out.clear();
         //n = UTF16Length(bytes);
         n = std::min(n, UTF16Length(bytes));
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
         out.reserve(n * 4);
         out.clear();
         //n = UTF32Length(bytes);
         n = std::min(n, UTF32Length(bytes));
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
   return EncodeUTF8(s, (s ? wcslen(s) : 0), out);
}

bool EncodeUTF8(const wchar_t *s, size_t len, std::string &out)
{
   switch (sizeof(wchar_t))
   {
   case 4:
      return EncodeUTF8(IsBigEndian() ? UTF_32BE : UTF_32LE, (const char*)s, 4 * len, out);
   case 2:
      return EncodeUTF8(IsBigEndian() ? UTF_16BE : UTF_16LE, (const char*)s, 2 * len, out);
   default:
      return false;
   }
}

bool DecodeUTF8(const char *s, Encoding e, std::string &out)
{
   return DecodeUTF8(s, UTF8Length((const Byte*)s), e, out);
}

bool DecodeUTF8(const char *s, size_t len, Encoding e, std::string &out)
{
   if (s)
   {
      size_t i = 0;
      size_t j = 0;
      char c8 = 0;
      Char16 c16;
      Char32 c32;
      const Byte *bytes = (const Byte*)s;
      //size_t n = UTF8Length(bytes);
      size_t n = std::min(len, UTF8Length(bytes));
      unsigned int cp = InvalidCodepoint;
      
      switch (e)
      {
      case UTF_8:
         //out = s;
         out.assign(s, n);
         return true;
      
      case ASCII:
         out.reserve(n);
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
      case ASCII_ISO_8859_2:
      case ASCII_ISO_8859_3:
      case ASCII_ISO_8859_4:
      case ASCII_ISO_8859_5:
      case ASCII_ISO_8859_6:
      case ASCII_ISO_8859_7:
      case ASCII_ISO_8859_8:
      case ASCII_ISO_8859_9:
      case ASCII_ISO_8859_10:
      case ASCII_ISO_8859_11:
      case ASCII_ISO_8859_13:
      case ASCII_ISO_8859_14:
      case ASCII_ISO_8859_15:
      case ASCII_ISO_8859_16:
         {
            InitASCIIReverseMappings();
            
            const gcore::HashMap<Codepoint, unsigned char> &rmapping = iso_8859_rev[e - ASCII_ISO_8859_1];
            
            out.reserve(n);
            out.clear();
            
            try
            {
               while (i < n)
               {
                  cp = DecodeUTF8(bytes, n, &i);
                  if (IsValidCodepoint(cp))
                  {
                     // rmapping[cp] will raise an exception if cp isn't mapped
                     out.push_back(char(rmapping[cp]));
                  }
                  else
                  {
                     return false;
                  }
               }
            }
            catch (std::exception &e)
            {
               std::cerr << e.what() << std::endl;
               return false;
            }
         }
         return true;
      
      case UCS_2BE:
      case UCS_2LE:
         out.reserve(n * 2);
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
         out.reserve(n * 2);
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
         out.reserve(n * 4);
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
   return DecodeUTF8(s, UTF8Length((const Byte*)s), out);
}

bool DecodeUTF8(const char *s, size_t len, std::wstring &out)
{
   std::string tmp;
   
   switch (sizeof(wchar_t))
   {
   case 4:
      if (!DecodeUTF8(s, len, IsBigEndian() ? UTF_32BE : UTF_32LE, tmp))
      {
         return false;
      }
      break;
   case 2:
      if (!DecodeUTF8(s, len, IsBigEndian() ? UTF_16BE : UTF_16LE, tmp))
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

#ifdef _WIN32

// CP_ACP  : Current code page
// CP_UTF8 : utf-8 
bool ToWideString(int codepage, const char *s, std::wstring &out)
{
   bool rv = false;
   
   if (s)
   {
      int slen = int(strlen(s));
      int wslen = MultiByteToWideChar(codepage, 0, s, slen, NULL, 0);
      wchar_t *ws = new wchar_t[wslen + 1];
      if (MultiByteToWideChar(codepage, 0, s, slen, ws, wslen + 1) != 0)
      {
         out = ws;
         rv = true;
      }
      delete[] ws;
   }
   
   if (!rv)
   {
      out.clear();
   }
   
   return false;
}

bool ToMultiByteString(const wchar_t *ws, int codepage, std::string &out)
{
   bool rv = false;
   
   if (ws)
   {
      int wslen = int(wcslen(ws));
      int slen = WideCharToMultiByte(codepage, 0, ws, wslen, NULL, 0, NULL, NULL);
      char *s = new char[slen + 1];
      if (WideCharToMultiByte(codepage, 0, ws, wslen, s, slen + 1, NULL, NULL) != 0)
      {
         out = s;
         rv = true;
      }
      delete[] s;
   }
   
   if (!rv)
   {
      out.clear();
   }
   
   return rv;
}

#endif

}
