/*
MIT License

Copyright (c) 2010 Gaetan Guidet

This file is part of gcore.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <gcore/string.h>
#include <gcore/path.h>

void QuoteString(gcore::String &s)
{
   s = "\"" + s + "\"";
}

int main(int, char**)
{
   gcore::String s0 = "Z:\\ve\\home/GaetanG/dev\\deploy";

   gcore::String s1 = s0 + "/\tPoo  ";
   s1.replace('\\', '/');

   gcore::String s2 = s1;
   s2.replace("GaetanG", "PetitN");

   gcore::StringList splits;
   s2.split('/', splits);
   for (size_t i=0; i<splits.size(); ++i)
   {
      splits[i].strip();
   }

   std::cout << "s0 = \"" << s0 << "\"" << std::endl;
   std::cout << "s1 = \"" << s1 << "\"" << std::endl;
   std::cout << "s2 = \"" << s2 << "\"" << std::endl;
   std::cout << "ToUpper s2:" << std::endl;
   std::cout << "   = \"" << s2.toupper() << "\"" << std::endl;
   std::cout << "ToLower s2:" << std::endl;
   std::cout << "   = \"" << s2.tolower() << "\"" << std::endl;
   std::cout << "Split Strip s1:" << std::endl;

   gcore::StringList::MapFunc quote;
   gcore::Bind(QuoteString, quote);
   std::cout << splits.map(quote) << std::endl;

   std::cout << "s0 = \"" << s0 << "\" starts with 'Z:' ? " << s0.startswith("Z:") << std::endl;
   std::cout << "s0 = \"" << s0 << "\" starts with 'z:' ? " << s0.startswith("z:") << std::endl;
   std::cout << "s0 = \"" << s0 << "\" ends with 'deploy' ? " << s0.endswith("deploy") << std::endl;
   std::cout << "s0 = \"" << s0 << "\" ends with 'Deploy' ? " << s0.endswith("Deploy") << std::endl;

   return 0;
}
