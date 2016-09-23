#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>

int main(int argc, char **argv)
{
   if (argc != 2)
   {
      std::cout << "Missing argument" << std::endl;
      return 1;
   }

   FILE *f = fopen(argv[1], "rb");
   if (!f)
   {
      std::cout << "Invalid file \"" << argv[1] << "\"" << std::endl;
      return 1;
   }
   
   unsigned char BOM[4];
   bool found = false;   
   size_t n = fread(BOM, 1, 8, f);
   
   if (n >= 2)
   {
      if (BOM[0] == 0xFF && BOM[1] == 0xFE)
      {
         if (n == 4 && BOM[2] == 0 && BOM[3] == 0)
         {
            std::cout << "UTF-32 LE" << std::endl;
         }
         else
         {
            std::cout << "UTF-16 LE" << std::endl;
         }
         found = true;
      }
      else if (BOM[0] == 0xFE && BOM[1] == 0xFF)
      {
         std::cout << "UTF-16 BE" << std::endl;
         found = true;
      }
      else if (n >= 3)
      {
         if (BOM[0] == 0xEF && BOM[1] == 0xBB && BOM[2] == 0xBF)
         {
            std::cout << "UTF-8" << std::endl;
            found = true;
         }
         else if (n == 4 && BOM[0] == 0 && BOM[1] == 0 && BOM[2] == 0xFE && BOM[3] == 0xFF)
         {
            std::cout << "UTF-32 BE" << std::endl;
            found = true;
         }
      }
   }
   
   if (!found)
   {
      std::cout << "No BOM found" << std::endl;
   }
   
   fclose(f);

   return 0;
}
