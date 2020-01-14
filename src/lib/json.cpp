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

#include <gcore/json.h>
#include <gcore/plist.h>
#include <gcore/unicode.h>

namespace gcore
{

namespace json
{

Value::Value()
   : mType(NullType)
{
}

Value::Value(Type t)
   : mType(t)
{
   switch (mType)
   {
   case BooleanType:
      mValue.boo = false;
      break;
   case NumberType:
      mValue.num = 0.0;
      break;
   case StringType:
      mValue.str = new String();
      break;
   case ObjectType:
      mValue.obj = new Object();
      break;
   case ArrayType:
      mValue.arr = new Array();
   default:
      break;
   }
}

Value::Value(bool b)
   : mType(BooleanType)
{
   mValue.boo = b;
}

Value::Value(int num)
   : mType(NumberType)
{
   mValue.num = double(num);
}

Value::Value(float num)
   : mType(NumberType)
{
   mValue.num = double(num);
}

Value::Value(double num)
   : mType(NumberType)
{
   mValue.num = num;
}

Value::Value(String *str)
   : mType(str ? StringType : NullType)
{
   mValue.str = str;
}

Value::Value(const char *str)
   : mType(str ? StringType : NullType)
{
   mValue.str = (str ? new String(str) : 0);
}

Value::Value(const wchar_t *wstr)
   : mType(wstr ? StringType : NullType)
{
   mValue.str = (wstr ? new String(wstr) : 0);
}

Value::Value(const String &str)
   : mType(StringType)
{
   mValue.str = new String(str);
}

Value::Value(Object *obj)
   : mType(obj ? ObjectType : NullType)
{
   mValue.obj = obj;
}

Value::Value(const Object &obj)
   : mType(ObjectType)
{
   mValue.obj = new Object(obj);
}

Value::Value(Array *arr)
   : mType(arr ? ArrayType : NullType)
{
   mValue.arr = arr;
}

Value::Value(const Array &arr)
   : mType(ArrayType)
{
   mValue.arr = new Array(arr);
}

Value::Value(const Value &rhs)
   : mType(rhs.mType)
{
   switch (mType)
   {
   case BooleanType:
      mValue.boo = rhs.mValue.boo;
      break;
   case NumberType:
      mValue.num = rhs.mValue.num;
      break;
   case StringType:
      mValue.str = new String(*(rhs.mValue.str));
      break;
   case ObjectType:
      mValue.obj = new Object(*(rhs.mValue.obj));
      break;
   case ArrayType:
      mValue.arr = new Array(*(rhs.mValue.arr));
   default:
      break;
   }
}

Value::~Value()
{
   reset();
}

bool Value::toPropertyList(PropertyList &pl) const
{
   if (mType != ObjectType)
   {
      return false;
   }
   
   pl.create();
   
   return toPropertyList(pl, "");
}

bool Value::toPropertyList(PropertyList &pl, const String &cprop) const
{
   switch (mType)
   {
   case ObjectType:
      {
         String prop, bprop = cprop;
         
         if (cprop.length() > 0)
         {
            bprop += ".";
         }
         
         for (Object::const_iterator it=objectBegin(); it!=objectEnd(); ++it)
         {
            prop = bprop + it->first;
            if (!it->second.toPropertyList(pl, prop))
            {
               return false;
            }
         }
      }
      break;
   
   case ArrayType:
      {
         String prop, bprop = cprop + "[";
         size_t i = 0;
         
         for (Array::const_iterator it=arrayBegin(); it!=arrayEnd(); ++it, ++i)
         {
            prop = bprop + String(i) + "]";
            if (!it->toPropertyList(pl, prop))
            {
               return false;
            }
         }
      }
      break;
   
   case BooleanType:
      if (!pl.setBoolean(cprop, mValue.boo))
      {
         return false;
      }
      break;
   
   case NumberType:
      if (!pl.setReal(cprop, mValue.num))
      {
         return false;
      }
      break;
   
   case StringType:
      if (!pl.setString(cprop, *(mValue.str)))
      {
         return false;
      }
      break;
   
   case NullType:
      // Ignore key
   default:
      break;
   }
   
   return true;
}

void Value::reset()
{
   switch (mType)
   {
   case StringType:
      delete mValue.str;
      mValue.str = 0;
      break;
   case ObjectType:
      delete mValue.obj;
      mValue.obj = 0;
      break;
   case ArrayType:
      delete mValue.arr;
      mValue.arr = 0;
   default:
      break;
   }
   
   mType = NullType;
}

Value& Value::operator=(bool b)
{
   if (mType != BooleanType)
   {
      reset();
      mType = BooleanType;
   }
   
   mValue.boo = b;
   
   return *this;
}

Value& Value::operator=(int num)
{
   if (mType != NumberType)
   {
      reset();
      mType = NumberType;
   }
   
   mValue.num = double(num);
   
   return *this;
}

Value& Value::operator=(float num)
{
   if (mType != NumberType)
   {
      reset();
      mType = NumberType;
   }
   
   mValue.num = double(num);
   
   return *this;
}

Value& Value::operator=(double num)
{
   if (mType != NumberType)
   {
      reset();
      mType = NumberType;
   }
   
   mValue.num = num;
   
   return *this;
}

Value& Value::operator=(String *str)
{
   if (mType != StringType)
   {
      reset();
      mType = StringType;
      mValue.str = 0;
   }
   
   if (str)
   {
      if (mValue.str && mValue.str != str)
      {
         delete mValue.str;
      }
      mValue.str = str;
   }
   else
   {
      if (mValue.str)
      {
         delete mValue.str;
         mValue.str = 0;
      }
      mType = NullType;
   }
   
   return *this;
}

Value& Value::operator=(const char *str)
{
   if (mType != StringType)
   {
      reset();
      mType = StringType;
      mValue.str = 0;
   }
   
   if (str)
   {
      if (mValue.str)
      {
         mValue.str->assign(str);
      }
      else
      {
         mValue.str = new String(str);
      }
   }
   else
   {
      if (mValue.str)
      {
         delete mValue.str;
         mValue.str = 0;
      }
      mType = NullType;
   }
   
   return *this;
}

Value& Value::operator=(const wchar_t *wstr)
{
   if (mType != StringType)
   {
      reset();
      mType = StringType;
      mValue.str = 0;
   }
   
   if (wstr)
   {
      if (mValue.str)
      {
         EncodeUTF8(wstr, *(mValue.str));
      }
      else
      {
         mValue.str = new String(wstr);
      }
   }
   else
   {
      if (mValue.str)
      {
         delete mValue.str;
         mValue.str = 0;
      }
      mType = NullType;
   }
   
   return *this;
}

Value& Value::operator=(const String &str)
{
   if (mType != StringType)
   {
      reset();
      mType = StringType;
      mValue.str = 0;
   }
   
   if (mValue.str)
   {
      mValue.str->assign(str);
   }
   else
   {
      mValue.str = new String(str);
   }
   
   return *this;
}

Value& Value::operator=(Object *obj)
{
   if (mType != ObjectType)
   {
      reset();
      mType = ObjectType;
      mValue.obj = 0;
   }
   
   if (obj)
   {
      if (mValue.obj && mValue.obj != obj)
      {
         delete mValue.obj;
      }
      mValue.obj = obj;
   }
   else
   {
      if (mValue.obj)
      {
         delete mValue.obj;
         mValue.obj = 0;
      }
      mType = NullType;
   }
   
   return *this;
}

Value& Value::operator=(const Object &obj)
{
   if (mType != ObjectType)
   {
      reset();
      mType = ObjectType;
      mValue.obj = 0;
   }
   
   if (mValue.obj)
   {
      *mValue.obj = obj;
   }
   else
   {
      mValue.obj = new Object(obj);
   }
   
   return *this;
}

Value& Value::operator=(Array *arr)
{
   if (mType != ArrayType)
   {
      reset();
      mType = ArrayType;
      mValue.arr = 0;
   }
   
   if (arr)
   {
      if (mValue.arr && mValue.arr != arr)
      {
         delete mValue.arr;
      }
      mValue.arr = arr;
   }
   else
   {
      if (mValue.arr)
      {
         delete mValue.arr;
         mValue.arr = 0;
      }
      mType = NullType;
   }
   
   return *this;
}

Value& Value::operator=(const Array &arr)
{
   if (mType != ArrayType)
   {
      reset();
      mType = ArrayType;
      mValue.arr = 0;
   }
   
   if (mValue.arr)
   {
      *mValue.arr = arr;
   }
   else
   {
      mValue.arr = new Array(arr);
   }
   
   return *this;
}

Value& Value::operator=(const Value &rhs)
{
   if (this != &rhs)
   {
      switch (rhs.mType)
      {
      case NullType:
         reset();
         break;
      case BooleanType:
         operator=(rhs.mValue.boo);
         break;
      case NumberType:
         operator=(rhs.mValue.num);
         break;
      case StringType:
         operator=(*(rhs.mValue.str));
         break;
      case ObjectType:
         operator=(*(rhs.mValue.obj));
         break;
      case ArrayType:
         operator=(*(rhs.mValue.arr));
         break;
      default:
         break;
      }
   }
   return *this;
}

Value::Type Value::type() const
{
   return mType;
}

Value::operator bool () const
{
   if (mType != BooleanType)
   {
      throw std::runtime_error("gcore::json::Value is not a boolean.");
   }
   return mValue.boo;
}

Value::operator int () const
{
   if (mType != NumberType)
   {
      throw std::runtime_error("gcore::json::Value is not a number.");
   }
   return int(mValue.num);
}

Value::operator float () const
{
   if (mType != NumberType)
   {
      throw std::runtime_error("gcore::json::Value is not a number.");
   }
   return float(mValue.num);
}

Value::operator double () const
{
   if (mType != NumberType)
   {
      throw std::runtime_error("gcore::json::Value is not a number.");
   }
   return mValue.num;
}

Value::operator const String& () const
{
   if (mType != StringType)
   {
      throw std::runtime_error("gcore::json::Value is not a string.");
   }
   return *(mValue.str);
}

Value::operator const char* () const
{
   if (mType != StringType)
   {
      throw std::runtime_error("gcore::json::Value is not a string.");
   }
   return mValue.str->c_str();
}

Value::operator const Object& () const
{
   if (mType != ObjectType)
   {
      throw std::runtime_error("gcore::json::Value is not an object.");
   }
   return *(mValue.obj);
}

Value::operator const Array& () const
{
   if (mType != ArrayType)
   {
      throw std::runtime_error("gcore::json::Value is not an array.");
   }
   return *(mValue.arr);
}

Value::operator String& ()
{
   if (mType != StringType)
   {
      throw std::runtime_error("gcore::json::Value is not a string.");
   }
   return *(mValue.str);
}

Value::operator Object& ()
{
   if (mType != ObjectType)
   {
      throw std::runtime_error("gcore::json::Value is not an object.");
   }
   return *(mValue.obj);
}

Value::operator Array& ()
{
   if (mType != ArrayType)
   {
      throw std::runtime_error("gcore::json::Value is not an array.");
   }
   return *(mValue.arr);
}

size_t Value::size() const
{
   if (mType == ArrayType)
   {
      return mValue.arr->size();
   }
   else if (mType == ObjectType)
   {
      return mValue.obj->size();
   }
   else
   {
      return 0;
   }
}

void Value::clear()
{
   if (mType == ArrayType)
   {
      mValue.arr->clear();
   }
   else if (mType == ObjectType)
   {
      mValue.obj->clear();
   }
}

bool Value::insert(size_t pos, const Value &value)
{
   if (mType != ArrayType)
   {
      return false;
   }
   mValue.arr->insert(mValue.arr->begin() + pos, value);
   return true;
}

bool Value::insert(const String &key, const Value &value)
{
   if (mType != ObjectType)
   {
      return false;
   }
   (*(mValue.obj))[key] = value;
   return true;
}

bool Value::erase(size_t pos, size_t cnt)
{
   if (mType != ArrayType)
   {
      return false;
   }
   size_t n = mValue.arr->size();
   if (pos >= n)
   {
      return false;
   }
   if (pos + cnt > n)
   {
      cnt = n - pos;
   }
   Array::iterator first = mValue.arr->begin() + pos;
   Array::iterator last = first + cnt;
   mValue.arr->erase(first, last);
   return true;
}

bool Value::erase(const String &key)
{
   if (mType != ObjectType)
   {
      return false;
   }
   Object::iterator it = mValue.obj->find(key);
   if (it != mValue.obj->end())
   {
      mValue.obj->erase(it);
      return true;
   }
   else
   {
      return false;
   }
}

static Array gsEmptyArray;
static Object gsEmptyObject;

ArrayConstIterator Value::arrayBegin() const
{
   return (mType == ArrayType ? mValue.arr->begin() : gsEmptyArray.begin());
}

ArrayConstIterator Value::arrayEnd() const
{
   return (mType == ArrayType ? mValue.arr->end() : gsEmptyArray.end());
}

ArrayIterator Value::arrayBegin()
{
   return (mType == ArrayType ? mValue.arr->begin() : gsEmptyArray.begin());
}

ArrayIterator Value::arrayEnd()
{
   return (mType == ArrayType ? mValue.arr->end() : gsEmptyArray.end());
}

const Value& Value::operator[](size_t idx) const
{
   if (mType != ArrayType)
   {
      throw std::runtime_error("gcore::json::Value is not an array.");
   }
   return mValue.arr->at(idx);
}

Value& Value::operator[](size_t idx)
{
   if (mType != ArrayType)
   {
      throw std::runtime_error("gcore::json::Value is not an array.");
   }
   return mValue.arr->at(idx);
}

ObjectConstIterator Value::objectBegin() const
{
   return (mType == ObjectType ? mValue.obj->begin() : gsEmptyObject.begin());
}

ObjectConstIterator Value::objectEnd() const
{
   return (mType == ObjectType ? mValue.obj->end() : gsEmptyObject.end());
}

ObjectConstIterator Value::find(const String &name) const
{
   return (mType == ObjectType ? mValue.obj->find(name) : gsEmptyObject.end());
}

ObjectConstIterator Value::find(const char *name) const
{
   if (mType != ObjectType)
   {
      return gsEmptyObject.end();
   }
   String _name(name);
   return this->find(_name);
}

ObjectConstIterator Value::find(const wchar_t *name) const
{
   if (mType != ObjectType)
   {
      return gsEmptyObject.end();
   }
   String _name(name);
   return this->find(_name);
}

ObjectIterator Value::objectBegin()
{
   return (mType == ObjectType ? mValue.obj->begin() : gsEmptyObject.begin());
}

ObjectIterator Value::objectEnd()
{
   return (mType == ObjectType ? mValue.obj->end() : gsEmptyObject.end());
}

ObjectIterator Value::find(const String &name)
{
   return (mType == ObjectType ? mValue.obj->find(name) : gsEmptyObject.end());
}

ObjectIterator Value::find(const char *name)
{
   if (mType != ObjectType)
   {
      return gsEmptyObject.end();
   }
   String _name(name);
   return this->find(_name);
}

ObjectIterator Value::find(const wchar_t *name)
{
   if (mType != ObjectType)
   {
      return gsEmptyObject.end();
   }
   String _name(name);
   return this->find(_name);
}

const Value& Value::operator[](const String &name) const
{
   if (mType != ObjectType)
   {
      throw std::runtime_error("gcore::json::Value is not an object.");
   }
   Object::const_iterator it = mValue.obj->find(name);
   if (it == mValue.obj->end())
   {
      throw std::out_of_range("Invalid key \"" + name + "\".");
   }
   return it->second;
}

Value& Value::operator[](const String &name)
{
   if (mType != ObjectType)
   {
      throw std::runtime_error("gcore::json::Value is not an object.");
   }
   return (*mValue.obj)[name];
}

const Value& Value::operator[](const char *name) const
{
   String _name(name);
   return this->operator[](_name);
}

const Value& Value::operator[](const wchar_t *name) const
{
   String _name(name);
   return this->operator[](_name);
}

Value& Value::operator[](const char *name)
{
   String _name(name);
   return this->operator[](_name);
}

Value& Value::operator[](const wchar_t *name)
{
   String _name(name);
   return this->operator[](_name);
}

Status Value::write(const Path &path, bool asciiOnly) const
{
   if (mType != ObjectType)
   {
      return Status(false, "Value is not an object.");
   }
   
   std::ofstream out;
   
   if (path.open(out))
   {
      write(out, asciiOnly);
      return Status(true);
   }
   else
   {
      return Status(false, "Invalid file '%s'", path.fullname('/').c_str());
   }
}

static void WriteASCIIString(std::ostream &os, const String &s)
{
   const char *c = s.c_str();
   const char *e = c + s.length();
   char tmp[8];
   
   while (*c != '\0')
   {
      if (IsUTF8SingleChar(*c))
      {
         os << *c;
      }
      else
      {
         Codepoint cp = DecodeUTF8(c, e - c);
         if (IsValidCodepoint(cp))
         {
            size_t n = CodepointToASCII(cp, ACF_16, tmp, 8);
            //os << "\\";
            for (size_t i=0; i<n; ++i)
            {
               os << tmp[i];
            }
         }
         else
         {
            std::cerr << "Skip invalid utf-8 character" << std::endl;
         }
      }
      c = UTF8Next(c);
   }
}

void Value::write(std::ostream &os, bool asciiOnly, const String indent, bool skipFirstIndent) const
{
   switch (type())
   {
   case NullType:
      os << (skipFirstIndent ? "" : indent) << "null";
      break;
   case BooleanType:
      os << (skipFirstIndent ? "" : indent) << (mValue.boo ? "true" : "false");
      break;
   case NumberType:
      os << (skipFirstIndent ? "" : indent) << mValue.num;
      break;
   case StringType:
      if (asciiOnly)
      {
         os << (skipFirstIndent ? "" : indent) << "\"";
         WriteASCIIString(os, *(mValue.str));
         os << "\"";
      }
      else
      {
         os << (skipFirstIndent ? "" : indent) << "\"" << mValue.str->c_str() << "\"";
      }
      break;
   case ObjectType:
      {
         size_t i=0, n=mValue.obj->size();
         os << (skipFirstIndent ? "" : indent) << "{" << std::endl;
         for (Object::const_iterator it=mValue.obj->begin(); it!=mValue.obj->end(); ++it, ++i)
         {
            if (asciiOnly)
            {
               os << indent << "  \"";
               WriteASCIIString(os, it->first);
               os << "\": ";
            }
            else
            {
               os << indent << "  \"" << it->first.c_str() << "\": ";
            }
            it->second.write(os, asciiOnly, indent + "  ", true);
            if (i + 1 < n) os << ", ";
            os << std::endl;
         }
         os << indent << "}";
      }
      break;
   case ArrayType:
      {
         size_t i=0, n=mValue.arr->size();
         os << (skipFirstIndent ? "" : indent) << "[" << std::endl;
         for (Array::const_iterator it=mValue.arr->begin(); it!=mValue.arr->end(); ++it, ++i)
         {
            it->write(os, asciiOnly, indent + "  ");
            if (i + 1 < n) os << ", ";
            os << std::endl;
         }
         os << indent << "]";
      }
      break;
   default:
      break;
   }
}

Status Value::read(const Path &path)
{
   std::ifstream in;
   
   if (!path.open(in))
   {
      reset();
      return Status(false, "Invalid file '%s'", path.fullname('/').c_str());
   }
   else
   {
      return read(in, true, 0);
   }
}

Status Value::read(std::istream &in)
{
   return read(in, false, 0);
}

Status Value::Parse(const Path &path, Value::ParserCallbacks &callbacks)
{
   json::Value val;
   
   std::ifstream in;
   
   if (path.open(in))
   {
      Status rv = val.read(in, true, &callbacks);
      val.reset();
      return rv;
   }
   else
   {
      return Status(false, "Invalid file '%s'", path.fullname('/').c_str());
   }
}

Status Value::Parse(std::istream &is, ParserCallbacks &callbacks)
{
   json::Value val;
   
   Status rv = val.read(is, true, &callbacks);
   
   val.reset();
   
   return rv;
}

struct ParserStackItem
{
   Value::ParserState state;
   Value *value;
   size_t count;
   
   inline ParserStackItem(Value::ParserState s)
      : state(s)
      , value(0)
      , count(0)
   {
   }
   
   inline ParserStackItem(Value::ParserState s, Value *v)
      : state(s)
      , value(v)
      , count(0)
   {
   }
};

static Status Failed(Value *value, size_t line, size_t col, const char *fmt, ...)
{
   value->reset();
   
   if (!fmt)
   {
      return Status(false, " (line %u, column %u)", line, col);
   }
   else
   {
      char buffer[4096];
      va_list args;
      va_start(args, fmt);
      vsnprintf(buffer, 4095, fmt, args);
      va_end(args);
      return Status(false, "%s (line %u, column %u)", buffer, line, col);
   }
}

Status Value::read(std::istream &in, bool consumeAll, Value::ParserCallbacks *cb)
{
   static const char *sSpaces = " \t\r\n";
   static const char *sNumberChars = "0123456789.eE+-";
   
   reset();
   
   if (!in.good())
   {
      return Status(false, "Invalid input stream.");
   }
   
   String remain, tmp;
   size_t p0, p1, lineno = 0, coloff = 0;
   
   std::vector<ParserStackItem> stack;
   ParserState state = Begin;
   String str = "";
   String key = "";
   bool readSep = true;
   bool hasSep = false;
   std::streampos orgPos = in.tellg();
   std::streampos lastReadPos = orgPos;
   
   while (in.good())
   {
      if (remain.length() == 0)
      {
         // Note: getline discards the trailing '\n'
         lastReadPos = in.tellg();
         std::getline(in, remain);
         #ifdef _DEBUG
         std::cout << "Parse| Read line '" << remain << "'" << std::endl;
         #endif
         ++lineno;
         coloff = 1;
      }
      
      switch (state)
      {
      case Begin:
         #ifdef _DEBUG
         std::cout << "Parse|Begin" << std::endl;
         #endif
         p0 = remain.find_first_not_of(sSpaces);
         
         if (p0 == String::npos)
         {
            remain = "";
         }
         else
         {
            if (remain[p0] != '{')
            {
               in.seekg(orgPos, in.beg);
               return Failed(this, lineno, coloff+p0, "Expect object at top level");
            }
            else
            {
               if (!cb)
               {
                  mType = ObjectType;
                  mValue.obj = new Object();
                  stack.push_back(ParserStackItem(ReadObject, this));
               }
               else if (cb->objectBegin)
               {
                  cb->objectBegin();
                  stack.push_back(ParserStackItem(ReadObject));
               }
               
               state = ReadObject;
               readSep = true;
               
               remain = remain.substr(p0 + 1);
               coloff += p0 + 1;
            }
         }
         
         break;
      
      case ReadObject:
         #ifdef _DEBUG
         std::cout << "Parse|ReadObject (remain = '" << remain << "')" << std::endl;
         #endif
         
         p0 = remain.find_first_not_of(sSpaces);
         
         if (readSep)
         {
            if (p0 != String::npos)
            {
               if (stack.size() == 0)
               {
                  in.seekg(orgPos, in.beg);
                  return Failed(this, lineno, coloff+p0, "Invalid parser state (read object)");
               }
               
               if (stack.back().count == 0)
               {
                  if (remain[p0] == ',')
                  {
                     in.seekg(orgPos, in.beg);
                     return Failed(this, lineno, coloff+p0, "Unexpected ,");
                  }
                  
                  hasSep = false;
               }
               else
               {
                  if (remain[p0] != ',' && remain[p0] != '}')
                  {
                     in.seekg(orgPos, in.beg);
                     return Failed(this, lineno, coloff+p0, "Expected , or }");
                  }
                  
                  hasSep = (remain[p0] == ',');
                  
                  if (hasSep)
                  {
                     p0 = remain.find_first_not_of(sSpaces, p0 + 1);
                  }
               }
               
               readSep = false;
            }
            else
            {
               hasSep = false;
            }
         }
         
         if (p0 == String::npos)
         {
            remain = "";
         }
         else if (remain[p0] == '}')
         {
            if (hasSep)
            {
               in.seekg(orgPos, in.beg);
               return Failed(this, lineno, coloff+p0, "Unexpected , before }");
            }
            
            if (stack.size() == 0)
            {
               in.seekg(orgPos, in.beg);
               return Failed(this, lineno, coloff+p0, "Un-matched }");
            }
            
            stack.pop_back();
            
            if (cb && cb->objectEnd)
            {
               cb->objectEnd();
            }
            
            if (stack.size() == 0)
            {
               if (consumeAll)
               {
                  state = End;
               }
               else
               {
                  // Reset stream position just after p0
                  // - p0 is relative to begining or 'remain'
                  // - coloff is the 1-based column number of the begining of 'remain'
                  //   => coloff + p0 - 1 is the offset in last read line which starts at 'lastReadPos'
                  // Next character position in stream is thus:
                  //   lastReadPos + (coloff - 1) + p0 + 1
                  in.seekg(lastReadPos + std::streamoff(coloff + p0));
                  return Status(true);
               }
            }
            else
            {
               state = stack.back().state;
               readSep = true;
               
               if (state != ReadObject && state != ReadArray)
               {
                  in.seekg(orgPos, in.beg);
                  return Failed(this, lineno, coloff+p0, "Parent value must be either an object or an array");
               }
            }
            
            remain = remain.substr(p0 + 1);
            coloff += p0 + 1;
         }
         else if (remain[p0] == '"')
         {
            state = ReadObjectKey;
            
            str = "";
            key = "";
            
            remain = remain.substr(p0 + 1);
            coloff += p0 + 1;
         }
         else
         {
            in.seekg(orgPos, in.beg);
            return Failed(this, lineno, coloff+p0, "Expect string value");
         }
         
         break;
      
      case ReadArray:
         #ifdef _DEBUG
         std::cout << "Parse|ReadArray (remain = '" << remain << "')" << std::endl;
         #endif
         
         p0 = remain.find_first_not_of(sSpaces);
         
         if (readSep)
         {
            if (p0 != String::npos)
            {
               if (stack.size() == 0)
               {
                  in.seekg(orgPos, in.beg);
                  return Failed(this, lineno, coloff+p0, "Invalid parser state (read array)");
               }
               
               if (stack.back().count == 0)
               {
                  if (remain[p0] == ',')
                  {
                     in.seekg(orgPos, in.beg);
                     return Failed(this, lineno, coloff+p0, "Unexpected ,");
                  }
                  
                  hasSep = false;
               }
               else
               {
                  if (remain[p0] != ',' && remain[p0] != ']')
                  {
                     in.seekg(orgPos, in.beg);
                     return Failed(this, lineno, coloff+p0, "Expected , or ]");
                  }
                  
                  hasSep = (remain[p0] == ',');
                  
                  if (hasSep)
                  {
                     p0 = remain.find_first_not_of(sSpaces, p0 + 1);
                  }
               }
               
               readSep = false;
            }
            else
            {
               hasSep = false;
            }
         }
         
         if (p0 == String::npos)
         {
            remain = "";
         }
         else if (remain[p0] == ']')
         {
            if (hasSep)
            {
               in.seekg(orgPos, in.beg);
               return Failed(this, lineno, coloff+p0, "Unexpected , before ]");
            }
            
            if (stack.size() == 0)
            {
               in.seekg(orgPos, in.beg);
               return Failed(this, lineno, coloff+p0, "Un-matched ]");
            }
            
            stack.pop_back();
            
            if (stack.size() == 0)
            {
               in.seekg(orgPos, in.beg);
               return Failed(this, lineno, coloff+p0, "Orphan array value");
            }
            else
            {
               if (cb && cb->arrayEnd)
               {
                  cb->arrayEnd();
               }
               
               state = stack.back().state;
               readSep = true;
               
               if (state != ReadObject && state != ReadArray)
               {
                  in.seekg(orgPos, in.beg);
                  return Failed(this, lineno, coloff+p0, "Parent value must be either an object or an array");
               }
               
               remain = remain.substr(p0 + 1);
               coloff += p0 + 1;
            }
         }
         else
         {
            state = ReadValue;
            remain = remain.substr(p0);
            coloff += p0;
         }
         
         break;
      
      case ReadObjectKey:
      case ReadString:
         #ifdef _DEBUG
         std::cout << "Parse|" << (state == ReadString ? "ReadString" : "ReadObjectKey") << " (remain = '" << remain << "')" << std::endl;
         #endif
         
         p0 = 0;
         p1 = remain.find('"', p0);
         
         while (p1 != String::npos)
         {
            if (p1 == 0)
            {
               break;
            }
            else if (remain[p1 - 1] != '\\')
            {
               str += remain.substr(p0, p1 - p0);
               remain = remain.substr(p1 + 1);
               coloff += p1 + 1;
               break;
            }
            else
            {
               p0 = p1 + 1;
               p1 = remain.find('"', p0);
            }
         }
         
         if (p1 == String::npos)
         {
            // couldn't find closing "
            str += remain + "\n";
            remain = "";
         }
         else
         {
            // Validate escape characters in str
            
            size_t len = str.length();
            size_t elen = 0;
            Codepoint cp = InvalidCodepoint;
            p0 = 0;
            p1 = str.find('\\', p0);
            
            while (p1 != String::npos)
            {
               if (p1 + 1 < len)
               {
                  switch (str[p1 + 1])
                  {
                  case '"':
                  case '\\':
                  case '/':
                  case 'b':
                  case 'f':
                  case 'n':
                  case 'r':
                  case 't':
                     // good
                     p0 = p1 + 2;
                     break;
                  case 'u':
                     cp = InvalidCodepoint;
                     if (ASCIIToCodepoint(str.c_str() + p1, cp) == 0)
                     {
                        in.seekg(orgPos, in.beg);
                        return Failed(this, lineno, coloff+p1, "Expected 4 digits after \\u escape character");
                     }
                     elen = EncodeUTF8(cp, (char*)(str.c_str() + p1), 6);
                     if (elen == 0)
                     {
                        in.seekg(orgPos, in.beg);
                        return Failed(this, lineno, coloff+p1, "Codepoint requires more than 6 utf-8 characters");
                     }
                     str.erase(p1 + elen, 6 - elen);
                     len = str.length();
                     p0 = p1 + elen;
                     break;
                  default:
                     in.seekg(orgPos, in.beg);
                     return Failed(this, lineno, coloff+p1, "Unsupported escape character: \\%c", str[p1 + 1]);
                  }
                  
                  p1 = str.find('\\', p0);
               }
               else
               {
                  // trailing '\'
                  in.seekg(orgPos, in.beg);
                  return Failed(this, lineno, coloff+p1, "Incomplete escape character");
               }
            }
            
            if (state == ReadObjectKey)
            {
               #ifdef _DEBUG
               std::cout << "Parse|ReadObjectKey -> " << str << std::endl;
               #endif
               
               key = str;
               
               if (cb && cb->objectKey)
               {
                  cb->objectKey(key);
               }
               
               p1 = remain.find_first_not_of(sSpaces);
               
               if (p1 == String::npos || remain[p1] != ':')
               {
                  in.seekg(orgPos, in.beg);
                  return Failed(this, lineno, coloff+p1, "Expected : after string value");
               }
               
               remain = remain.substr(p1 + 1);
               
               state = ReadValue;
            }
            else
            {
               if (stack.size() == 0)
               {
                  in.seekg(orgPos, in.beg);
                  return Failed(this, lineno, coloff, "Orphan string");
               }
               
               ParserStackItem &psi = stack.back();
               
               psi.count++;
               
               if (cb && cb->stringScalar)
               {
                  cb->stringScalar(str);
               }
               
               if (psi.value)
               {
                  if (psi.value->type() == ObjectType)
                  {
                     if (key.length() == 0)
                     {
                        in.seekg(orgPos, in.beg);
                        return Failed(this, lineno, coloff, "Undefined or empty object member name");
                     }
                     (*(psi.value))[key] = str;
                     // reset key
                     key = "";
                  }
                  else if (psi.value->type() == ArrayType)
                  {
                     psi.value->insert(psi.value->size(), Value(str));
                  }
                  else
                  {
                     in.seekg(orgPos, in.beg);
                     return Failed(this, lineno, coloff, "Parent value must be either an object or an array");
                  }
               }
               
               state = psi.state;
               readSep = true;
            }
         }
         
         break;
      
      case ReadValue:
         #ifdef _DEBUG
         std::cout << "Parse|ReadValue (remain = '" << remain << "')" << std::endl;
         #endif
         
         p0 = remain.find_first_not_of(sSpaces);
         
         if (p0 == String::npos)
         {
            remain = "";
         }
         else
         {
            if (stack.size() == 0)
            {
               in.seekg(orgPos, in.beg);
               return Failed(this, lineno, coloff+p0, "Orphan value");
            }
            
            ParserStackItem &psi = stack.back();
            
            if (psi.state == ReadObject)
            {
               if (key.length() == 0)
               {
                  in.seekg(orgPos, in.beg);
                  return Failed(this, lineno, coloff+p0, "Undefined or empty object member name");
               }
            }
            else if (psi.state != ReadArray)
            {
               in.seekg(orgPos, in.beg);
               return Failed(this, lineno, coloff+p0, "Parent value must be either an object or an array");
            }
            
            if (remain[p0] == '"')
            {
               state = ReadString;
               str = "";
               remain = remain.substr(p0 + 1);
               coloff += p0 + 1;
            }
            else
            {
               Value *value = 0;
               
               psi.count++;
               
               if (psi.value)
               {
                  // add new object in 
                  if (psi.state == ReadObject)
                  {
                     value = &(*(psi.value))[key];
                     // reset key
                     key = "";
                  }
                  else
                  {
                     size_t idx = psi.value->size();
                     psi.value->insert(idx, Value());
                     value = &(*(psi.value))[idx];
                  }
               }
               
               if (remain[p0] == '{')
               {
                  if (cb && cb->objectBegin)
                  {
                     cb->objectBegin();
                  }
                  
                  if (value)
                  {
                     *value = Object();
                  }
                  
                  stack.push_back(ParserStackItem(ReadObject, value));
                  
                  state = ReadObject;
                  readSep = true;
                  
                  remain = remain.substr(p0 + 1);
                  coloff += p0 + 1;
               }
               else if (remain[p0] == '[')
               {
                  if (cb && cb->arrayBegin)
                  {
                     cb->arrayBegin();
                  }
                  
                  if (value)
                  {
                     *value = Array();
                  }
                  
                  stack.push_back(ParserStackItem(ReadArray, value));
                  
                  state = ReadArray;
                  readSep = true;
                  
                  remain = remain.substr(p0 + 1);
                  coloff += p0 + 1;
               }
               else
               {
                  if (!strncmp(remain.c_str() + p0, "null", 4))
                  {
                     if (cb && cb->nullScalar)
                     {
                        cb->nullScalar();
                     }
                     remain = remain.substr(p0 + 4);
                     coloff += p0 + 4;
                  }
                  else if (!strncmp(remain.c_str() + p0, "true", 4))
                  {
                     if (cb && cb->booleanScalar)
                     {
                        cb->booleanScalar(true);
                     }
                     if (value)
                     {
                        *value = true;
                     }
                     remain = remain.substr(p0 + 4);
                     coloff += p0 + 4;
                  }
                  else if (!strncmp(remain.c_str() + p0, "false", 5))
                  {
                     if (cb && cb->booleanScalar)
                     {
                        cb->booleanScalar(false);
                     }
                     if (value)
                     {
                        *value = false;
                     }
                     remain = remain.substr(p0 + 5);
                     coloff += p0 + 5;
                  }
                  else
                  {
                     // must be a number
                     String numstr;
                     
                     p1 = remain.find_first_not_of(sNumberChars, p0);
                     
                     if (p1 == String::npos)
                     {
                        numstr = remain.substr(p0);
                        remain = "";
                        coloff += p0;
                     }
                     else
                     {
                        numstr = remain.substr(p0, p1 - p0);
                        remain = remain.substr(p1);
                        coloff += p1;
                     }
                     
                     double val = 0.0;
                     
                     if (sscanf(numstr.c_str(), "%lf", &val) != 1)
                     {
                        in.seekg(orgPos, in.beg);
                        return Failed(this, lineno, coloff, "Expected number value");
                     }
                     
                     if (cb && cb->numberScalar)
                     {
                        cb->numberScalar(val);
                     }
                     if (value)
                     {
                        *value = val;
                     }
                  }
                  
                  // at this point we know stack is not empty
                  // and that last element is either an object or an array
                  state = psi.state;
                  readSep = true;
               }
            }
         }
         
         break;
      
      case End:
         #ifdef _DEBUG
         std::cout << "Parse|End (remain = '" << remain << "')" << std::endl;
         #endif
         
         // p0 = remain.find_first_not_of(sSpaces);
         // if (p0 != String::npos)
         // {
         //    in.seekg(orgPos, in.beg);
         //    return Failed(this, lineno, coloff+p0, "Content after top level object");
         // }
         
         remain = "";
         
      default:
         break;
      }
   }
   
   if (stack.size() != 0)
   {
      // in.good() is necessarily false
      // need to clear error state before restoring stream position
      in.clear();
      in.seekg(orgPos, in.beg);
      reset();
      return Status(false, "Incomplete JSON stream");
   }
   else
   {
      return Status(true);
   }
}

} // json

} // gcore

std::ostream& operator<<(std::ostream &os, const gcore::json::Value &value)
{
   switch (value.type())
   {
   case gcore::json::Value::NullType:
      os << "null";
      break;
   case gcore::json::Value::BooleanType:
      os << (bool(value) ? "true" : "false");
      break;
   case gcore::json::Value::NumberType:
      os << double(value);
      break;
   case gcore::json::Value::StringType:
      os << "\"" << (const char*)value << "\"";
      break;
   case gcore::json::Value::ObjectType:
      os << (const gcore::json::Object&)value;
      break;
   case gcore::json::Value::ArrayType:
      os << (const gcore::json::Array&)value;
      break;
   default:
      break;
   }
   return os;
}

std::ostream& operator<<(std::ostream &os, const gcore::json::Object &object)
{
   os << "{";
   size_t i = 0;
   size_t n = object.size();
   gcore::json::Object::const_iterator itend = object.end();
   for (gcore::json::Object::const_iterator it = object.begin(); it != itend; ++it, ++i)
   {
      os << "\"" << it->first.c_str() << "\": " << it->second;
      if (i + 1 < n)
      {
         os << ", ";
      }
   }
   os << "}";
   return os;
}

std::ostream& operator<<(std::ostream &os, const gcore::json::Array &array)
{
   os << "[";
   size_t i = 0;
   size_t n = array.size();
   gcore::json::Array::const_iterator itend = array.end();
   for (gcore::json::Array::const_iterator it = array.begin(); it != itend; ++it, ++i)
   {
      os << *it;
      if (i + 1 < n)
      {
         os << ", ";
      }
   }
   os << "]";
   return os;
}
