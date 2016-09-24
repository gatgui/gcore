#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>

int main(int argc, char **argv)
{
   if (argc != 3)
   {
      std::cout << "writebom <path> utf-8|utf-16|utf-16-be|utf-16-le|utf-32|utf-32-be|utf-32-le|none" << std::endl;
      return 1;
   }
   
   unsigned char BOM[4];
   size_t bs = 0;
   bool remove = false;

   if (!strcmp(argv[2], "utf-8"))
   {
      // utf-8 encoding of 0xFEFF
      BOM[0] = 0xEF;
      BOM[1] = 0xBB;
      BOM[2] = 0xBF;
      bs = 3;
   }
   else if (!strcmp(argv[2], "utf-16") ||
            !strcmp(argv[2], "utf-16-be"))
   {
      BOM[0] = 0xFE;
      BOM[1] = 0xFF;
      bs = 2;
   }
   else if (!strcmp(argv[2], "utf-16-le"))
   {
      BOM[0] = 0xFF;
      BOM[1] = 0xFE;
      bs = 2;
   }
   else if (!strcmp(argv[2], "utf-32") ||
            !strcmp(argv[2], "utf-32-be"))
   {
      BOM[0] = 0x00;
      BOM[1] = 0x00;
      BOM[2] = 0xFE;
      BOM[3] = 0xFF;
      bs = 4;
   }
   else if (!strcmp(argv[2], "utf-32-le"))
   {
      BOM[0] = 0xFF;
      BOM[1] = 0xFE;
      BOM[2] = 0x00;
      BOM[3] = 0x00;
      bs = 4;
   }
   else if (!strcmp(argv[2], "none"))
   {
      remove = true;
   }
   else
   {
      std::cout << "Invalid BOM \"" << argv[2] << "\"" << std::endl;
      return 1;
   }
   
   FILE *f = fopen(argv[1], "rb");
   if (!f)
   {
      std::cout << "Invalid file \"" << argv[1] << "\"" << std::endl;
      return 1;
   }
   
   unsigned char curBOM[4];
   size_t cbs = 0;
   size_t n = fread(curBOM, 1, 8, f);
   for (size_t i=0; i<n; ++i)
   {
      std::cout << std::hex << (unsigned int)BOM[i] << std::dec << " ";
   }
   std::cout << std::endl;
   
   if (n >= 2)
   {
      if (curBOM[0] == 0xFF && curBOM[1] == 0xFE)
      {
         if (n == 4 && curBOM[2] == 0 && curBOM[3] == 0)
         {
            cbs = 4;
         }
         else
         {
            cbs = 2;
         }
      }
      else if (curBOM[0] == 0xFE && curBOM[1] == 0xFF)
      {
         cbs = 2;
      }
      else if (n >= 3)
      {
         if (curBOM[0] == 0xEF && curBOM[1] == 0xBB && curBOM[2] == 0xBF)
         {
            cbs = 3;
         }
         else if (n == 4 && curBOM[0] == 0 && curBOM[1] == 0 && curBOM[2] == 0xFE && curBOM[3] == 0xFF)
         {
            cbs = 4;
         }
      }
   }
   
   if (!remove)
   {
      if (cbs == 0)
      {
         // there we go read the whole file as binary and insert bom
         fseek(f, 0, SEEK_END);
         size_t fs = size_t(ftell(f));
         unsigned char *content = (unsigned char*) malloc(fs);
         
         fseek(f, 0, SEEK_SET);
         if (fread(content, 1, fs, f) == fs)
         {
            fclose(f);
            
            f = fopen(argv[1], "wb");
            // doesn't work
            fwrite(BOM, 1, bs, f);
            fwrite(content, 1, fs, f);
         }
         else
         {
            std::cout << "Failed to read file content" << std::endl;
         }
      }
      else
      {
         std::cout << "BOM already set" << std::endl;
      }
   }
   else
   {
      if (cbs > 0)
      {
         fseek(f, 0, SEEK_END);
         size_t fs = size_t(ftell(f));
         unsigned char *content = (unsigned char*) malloc(fs);
         
         fseek(f, 0, SEEK_SET);
         if (fread(content, 1, fs, f) == fs)
         {
            fclose(f);
            f = fopen(argv[1], "wb");
            fwrite(content + cbs, 1, fs - cbs, f);
         }
         else
         {
            std::cout << "Failed to read file content" << std::endl;
         }
      }
      else
      {
         std::cout << "No BOM to remove" << std::endl;
      }
   }
   
   fclose(f);

   return 0;
}
