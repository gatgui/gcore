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

#include <gcore/json.h>
#include <gcore/plist.h>

using namespace gcore;

class Parser
{
public:
   Parser()
      : mDepth(0)
      , mIndent("  ")
   {
      Bind(this, METHOD(Parser, objectBegin), mCallbacks.objectBegin);
      Bind(this, METHOD(Parser, objectKey), mCallbacks.objectKey);
      Bind(this, METHOD(Parser, objectEnd), mCallbacks.objectEnd);
      Bind(this, METHOD(Parser, arrayBegin), mCallbacks.arrayBegin);
      Bind(this, METHOD(Parser, arrayEnd), mCallbacks.arrayEnd);
      Bind(this, METHOD(Parser, booleanScalar), mCallbacks.booleanScalar);
      Bind(this, METHOD(Parser, numberScalar), mCallbacks.numberScalar);
      Bind(this, METHOD(Parser, stringScalar), mCallbacks.stringScalar);
      Bind(this, METHOD(Parser, nullScalar), mCallbacks.nullScalar);
   }
   
   ~Parser()
   {
   }
   
   void objectBegin()
   {
      std::cout << (mIndent * mDepth) << "Object begin" << std::endl;
      ++mDepth;
   }
   
   void objectKey(const char *name)
   {
      std::cout << (mIndent * mDepth) << "Object key: '" << name << "'" << std::endl;
   }
   
   void objectEnd()
   {
      --mDepth;
      std::cout << (mIndent * mDepth) << "Object end" << std::endl;
   }
   
   void arrayBegin()
   {
      std::cout << (mIndent * mDepth) << "Array begin" << std::endl;
      ++mDepth;
   }
   
   void arrayEnd()
   {
      --mDepth;
      std::cout << (mIndent * mDepth) << "Array end" << std::endl;
   }
   
   void booleanScalar(bool v)
   {
      std::cout << (mIndent * mDepth) << "Boolean scalar: " << v << std::endl;
   }
   
   void numberScalar(double v)
   {
      std::cout << (mIndent * mDepth) << "Number scalar: " << v << std::endl;
   }
   
   void stringScalar(const char *v)
   {
      std::cout << (mIndent * mDepth) << "String scalar: " << v << std::endl;
   }
   
   void nullScalar()
   {
      std::cout << (mIndent * mDepth) << "Null scalar" << std::endl;
   }
   
   Status parse(const char *path)
   {
      mDepth = 0;
      return json::Value::Parse(path, mCallbacks);
   }

private:
   json::Value::ParserCallbacks mCallbacks;
   long mDepth;
   gcore::String mIndent;
};

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
      
      Status stat;
      
      stat = top.read(path);
      if (stat)
      {
         std::cout << "Succeeded" << std::endl;
         
         top.write(std::cout);
         std::cout << std::endl;
         
         PropertyList pl;
         if (top.toPropertyList(pl))
         {
            pl.write("out.xml");
         }
      }
      else
      {
         std::cout << "Failed: " << stat << std::endl;
      }
      
      Parser parser;
      
      stat = parser.parse(path);
      if (!stat)
      {
         std::cout << "Failed: " << stat << std::endl;
      }
   }
   
   return 0;
}
