// -*- coding: utf-8 -*-
#include <gcore/encoding.h>

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

int main(int, char **)
{
   std::cout << "sizeof(wchar_t)=" << sizeof(wchar_t) << std::endl;
   std::cout << "sizeof(unsigned int)=" << sizeof(unsigned int) << std::endl;
   std::cout << "IsBigEndian: " << gcore::IsBigEndian() << std::endl;
   std::cout << std::endl;

   // On linux/osx, the encoding for narrow string literals is that of the source file
   //               wide strings are UTF-32
   // On windows, at least if _UNICODE is not defined, narrow string will be encoded in current codepage
   //               wide strings are UTF-16
   // Also, for this test to be properly compiled, cl.exe must be able to figure out the encoding
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
   
   std::string utf8;
   std::wstring wstr;
   std::string utf16;
   for (int i=0; i<6; ++i)
   {
      std::cout << "tests[" << i << "]" << std::endl;
      PrintBytes(std::cout, tests[i], wcslen(tests[i]) * sizeof(wchar_t), 4) << std::endl;
      if (gcore::EncodeUTF8(tests[i], utf8))
      {
         std::cout << tests[i] << " -> utf-8 -> [" << utf8.length() << "] \"" << utf8 << "\"" << std::endl;
         PrintBytes(std::cout, utf8.c_str(), utf8.length(), 1) << std::endl;
         
         if (!gcore::DecodeUTF8(utf8.c_str(), wstr) || wstr != tests[i])
         {
            std::cerr << "Re-encoding failed" << std::endl;
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
            std::cout << gcore::EncodingString(encoding) << ": " << utf8 << std::endl;
            
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
   
   return 0;
}

