/*

Copyright (C) 2016  Gaetan Guidet

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

#include <gcore/gcore>

using namespace gcore;

int main(int argc, char **argv)
{
   json::Object top;
   json::Array ary1;
   json::Array ary2;
   json::Object obj1;
   json::Object obj2;
   json::Object obj3;
   
   ary1.push_back("Hello");
   ary1.push_back("World");
   ary1.push_back("Goodbye!");
   
   gcore::String val = ary1[1];
   std::cout << "Second element: " << val << std::endl;
   
   obj1["name"] = "James";
   obj1["age"] = 35;
   
   obj2["name"] = "Philip";
   obj2["age"] = 21;
   
   obj3["name"] = "Chloe";
   obj3["age"] = 27;
   
   ary2.push_back(obj1);
   ary2.push_back(obj2);
   
   top["myflt"] = json::Value(10.0f);
   top["myarray"] = ary1;
   top["objarray"] = ary2;
   
   std::cout << "Is a number? " << (top["myflt"].type() == json::Value::NumberType) << std::endl;
   std::cout << top["objarray"][1]["name"] << std::endl;
   std::cout << top << std::endl;
   
   // --- generic access ---
   
   json::Value all(top);
   json::Value &v = all["objarray"];
   
   json::Array &_v = all["objarray"];
   _v.push_back(obj3);
   
   v.insert(0, obj3);
   v.erase(3);
   
   for (json::ArrayConstIterator ait=v.abegin(); ait!=v.aend(); ++ait)
   {
      std::cout << "[" << (ait - v.abegin()) << "]" << std::endl;
      for (json::ObjectConstIterator oit=ait->obegin(); oit!=ait->oend(); ++oit)
      {
         std::cout << "  " << oit->first << ": " << oit->second << std::endl;
      }
   }
   
   if (argc > 1)
   {
      const char *path = argv[1];
      
      std::cout << "Read '" << path << "'..." << std::endl;
      
      json::Value top;
      
      try
      {
         top.read(path);
         std::cout << "Succeeded" << std::endl;
         
         top.write(std::cout);
         std::cout << std::endl;
         
         PropertyList pl;
         if (top.toPropertyList(pl))
         {
            pl.write("out.xml");
         }
      }
      catch (json::ParserError &e)
      {
         std::cout << "Failed: " << e.what() << std::endl;
      }
   }
   
   return 0;
}
