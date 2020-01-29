// -*- coding: utf-8 -*-
#include <gcore/unicode.h>
#include <gcore/argparser.h>
#include <gcore/path.h>

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

int main(int argc, char **argv)
{
   gcore::FlagDesc flags[1] = {ACCEPTS_NOFLAG_ARGUMENTS(-1)};
   gcore::ArgParser args(flags, 1);
   
   gcore::Status stat = args.parse(argc, argv);
   if (stat)
   {
      size_t n = args.argumentCount();
      gcore::String arg;
      for (size_t i=0; i<n; ++i)
      {
         args.getArgument(i, arg);
         std::cout << "arg[" << i << "] = " << arg << " (ascii: " << gcore::IsASCII(arg.c_str())<< ", utf-8: " << gcore::IsUTF8(arg.c_str()) << ")" << std::endl;
      }
   }
   else
   {
      std::cerr << stat << std::endl;
   }
   
   std::cout << "sizeof(wchar_t)=" << sizeof(wchar_t) << std::endl;
   std::cout << "sizeof(unsigned int)=" << sizeof(unsigned int) << std::endl;
   std::cout << "IsBigEndian: " << gcore::IsBigEndian() << std::endl;
   std::cout << std::endl;

   // On linux/osx, the encoding for narrow string literals is that of the source file
   //               wide strings are UTF-32
   // On windows, narrow string will be encoded in the current codepage
   //             wide strings are UTF-16
   // Also, for this test to be properly compiled, cl.exe must be able to figure out the encoding of the file
   //   in order to be able to transcode into current codepage. If it can't, current codepage encoding is
   //   assumed...
   // => use BOM even for UTF-8
   // BOMs
   //    UTF-32be  0x00 0x00 0xFE 0xFF
   //    UTF-32le  0xFF 0xFE 0x00 0x00
   //    UTF-16be  0xFE 0xFF
   //    UTF-16le  0xFF 0xFE 
   //    UTF-8     0xEF 0xBB 0xBF 

   const wchar_t *tests[6] = {
      L"片道",
      L"à",
      L"bien sûr.",
      L"Dans l’idéal évidement.",
      L"Si cela pouvait être implémenté proprement.",
      L"Plein de gros becs à mes petites chéries et aussi à la grande."
   };

   size_t len = wcslen(tests[0]) * sizeof(wchar_t);

   PrintBytes(std::cout, tests[0], len) << std::endl;
   std::cout << std::hex << "0x" << tests[0][0] << " 0x" << tests[0][1] << std::dec << std::endl;
   std::cout << std::endl;
   
   std::string str;
   std::string utf8;
   std::wstring wstr;
   std::string utf16;
   for (int i=0; i<6; ++i)
   {
      std::cout << "tests[" << i << "]" << std::endl;
      PrintBytes(std::cout, tests[i], wcslen(tests[i]) * sizeof(wchar_t), 4) << std::endl;
      if (gcore::EncodeUTF8(tests[i], utf8))
      {
         str = utf8;
         std::cout << tests[i] << " -> utf-8 -> [" << utf8.length() << "] \"" << str << "\"" << std::endl;
         PrintBytes(std::cout, utf8.c_str(), utf8.length(), 1) << std::endl;
         
         if (!gcore::DecodeUTF8(utf8.c_str(), wstr))
         {
            std::cerr << "Re-encoding failed (DecodeUTF8 failure)" << std::endl;
         }
         else if (wstr != tests[i])
         {
            std::cerr << "Re-encoding failed (Ouptut differs)" << std::endl;
            size_t l0 = wcslen(tests[i]);
            size_t l1 = wstr.length();
            if (l0 != l1)
            {
               std::cerr << "  Length: " << l0 << " -> " << l1 << std::endl;
               if (l1 > l0)
               {
                  for (size_t j=l0; j<l1; ++j)
                  {
                     std::cerr << "  +[" << j << "]: " << std::hex << size_t(wstr[j]) << std::dec << std::endl;
                  }
               }
            }
            else
            {
               for (size_t j=0; j<l0; ++j)
               {
                  if (wstr[j] != tests[i][j])
                  {
                     std::cerr << "  [" << j << "]: " << std::hex << size_t(tests[i][j]) << std::dec << " -> " << std::hex << size_t(wstr[j]) << std::dec << std::endl;
                  }
               }
            }
         }
         
         if (gcore::DecodeUTF8(utf8.c_str(), gcore::UTF_16, utf16))
         {
            std::cout << "-> utf-16 -> ";
            PrintBytes(std::cout, utf16.c_str(), utf16.length(), 2) << std::endl;
         }
      }
      std::cout << std::endl;
   }
   
   FILE *f = fopen("testdata/iso8859.txt", "r");
   if (f)
   {
      char buffer[256];
      fgets(buffer, 256, f);
      fclose(f);
      
      size_t len = strlen(buffer);
      if (len > 0 && buffer[len-1] == '\n')
      {
         buffer[len-1] = '\0';
      }
      
      for (int i=0; i<15; ++i)
      {
         // should cleanup string from all unsupported character
         gcore::Encoding encoding = gcore::Encoding(gcore::ASCII_ISO_8859_1 + i);
         if (gcore::EncodeUTF8(encoding, buffer, utf8))
         {
            str = utf8;
            std::cout << gcore::EncodingToString(encoding) << ": " << str << std::endl;
            
            std::string ascii;
            if (gcore::DecodeUTF8(utf8.c_str(), encoding, ascii))
            {
               if (ascii != buffer)
               {
                  // only fails if size is different or letter that mismatched as the replacement char 0xFFFD
                  std::cout << "!!! Round trip failed (decoding error)" << std::endl;
                  std::cout << "length (org): " << strlen(buffer) << std::endl;
                  std::cout << "length (dec): " << ascii.length() << std::endl;
                  for (size_t j=0; j<ascii.length(); ++j)
                  {
                     if (ascii[j] != buffer[j])
                     {
                        std::cout << "[" << j << "]: " << std::hex << (int(buffer[j]) & 0x00FF) << std::dec << " != "
                                                       << std::hex << (int(ascii[j]) & 0x00FF) << std::dec << std::endl;
                     }
                  }
               }
            }
            else
            {
               std::cout << "!!! Round trip failed (can't decode)" << std::endl;
            }
         }
      }
   }
   
   gcore::Codepoint cps[3] = {0x000128CC, 0x0000F4CE, 0x01FFFFFF};
   gcore::Codepoint cp0, cp1;
   char out[16];
   size_t n;
   
   for (int i=0; i<3; ++i)
   {
      cp0 = cps[i];
      
      for (int j=0; j<3; ++j)
      {
         n = gcore::CodepointToASCII(cp0, gcore::ASCIICodepointFormat(j), out, 15);
         if (n > 0)
         {
            out[n] = '\0';
            std::cout << "0x" << std::hex << cp0 << std::dec << " -> " << out << std::endl;
            
            n = gcore::ASCIIToCodepoint(out, cp1);
            if (n > 0)
            {
               if (cp1 != cp0)
               {
                  std::cout << "Bad value, got 0x" << std::hex << cp1 << std::dec << " expected 0x" << std::hex << cp0 << std::dec << std::endl;
               }
               else
               {
                  std::cout << out << " -> " << "0x" << std::hex << cp0 << std::dec << std::endl;
               }
            }
            else
            {
               std::cout << "Failed to convert ascii " << out << " to codepoint" << std::endl;
            }
         }
         else
         {
            std::cout << "Failed to convert codepoint 0x" << std::hex << cp0 << std::dec << " to ascii" << std::endl;
         }
      }
   }
   
   n = gcore::ASCIIToCodepoint("\\u01AC9", cp0);
   std::cout << "\\u01AC9 -> (" << n << ") 0x" << std::hex << cp0 << std::dec << std::endl;
   
   n = gcore::ASCIIToCodepoint("\\uA81", cp0);
   std::cout << "\\uA81 -> (" << n << ") 0x" << std::hex << cp0 << std::dec << std::endl;
   
   n = gcore::ASCIIToCodepoint("\\U001F01AC9", cp0);
   std::cout << "\\U001F01AC9 -> (" << n << ") 0x" << std::hex << cp0 << std::dec << std::endl;
   
   n = gcore::ASCIIToCodepoint("\\U1F0A81", cp0);
   std::cout << "\\U1F0A81 -> (" << n << ") 0x" << std::hex << cp0 << std::dec << std::endl;
   
   n = gcore::ASCIIToCodepoint("\\u{0001CA}", cp0);
   std::cout << "\\u{0001CA} -> (" << n << ") 0x" << std::hex << cp0 << std::dec << std::endl;
   
   n = gcore::ASCIIToCodepoint("\\u{0001FFFFF}", cp0);
   std::cout << "\\u{0001FFFFF} -> (" << n << ") 0x" << std::hex << cp0 << std::dec << std::endl;
   
   n = gcore::ASCIIToCodepoint("\\u{1FFFFF000}", cp0);
   std::cout << "\\u{1FFFFF000} -> (" << n << ") 0x" << std::hex << cp0 << std::dec << std::endl;
   
   const char *test = "\u0F1C - \U00000F1C";
   std::cout << test << " ('\\u0F1C - \\U00000F1C'): ";
   PrintBytes(std::cout, test, strlen(test), 1) << std::endl;
   
   n = gcore::EncodeUTF8(0xF1C, out, 16);
   out[n] = '\0';
   std::cout << "UTF-8 for codepoint 0xF1C: ";
   PrintBytes(std::cout, out, n, 1) << std::endl;
   
   
   gcore::Path path(L"testdata/問題のファイル.txt");
   std::ifstream inf;
   
   if (!path.open(inf))
   {
      std::cout << "Cannot open file with unicode name." << std::endl;
   }
   else
   {
      gcore::String line;
      
      while (inf.good())
      {
         std::getline(inf, line);
         gcore::UTF8ToLocale_ip(line);
         std::cout << line << std::endl;
      }
   }
   
   
   gcore::String nps(L"感じ"); // u'\u611f\u3058'
   gcore::String ps;
   gcore::String nps2;
   gcore::EncodeNonPrintableChars(nps.c_str(), ps);
   gcore::DecodeNonPrintableChars(ps.c_str(), nps2);
   std::cout << nps << " -> " << ps << " -> " << nps2 << " (round strip success? " << (nps == nps2) << ")" << std::endl;
   
   nps = "hello world!";
   gcore::EncodeNonPrintableChars(nps.c_str(), ps);
   gcore::DecodeNonPrintableChars(ps.c_str(), nps2);
   std::cout << nps << " -> " << ps << " -> " << nps2 << " (round strip success? " << (nps == nps2) << ")" << std::endl;
   
   
   return 0;
}

