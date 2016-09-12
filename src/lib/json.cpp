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

gcore::json::Value::Value()
   : mType(NullType)
{
}

gcore::json::Value::Value(Type t)
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
      mValue.str = new gcore::String();
      break;
   case ObjectType:
      mValue.obj = new gcore::json::Object();
      break;
   case ArrayType:
      mValue.arr = new gcore::json::Array();
   default:
      break;
   }
}

gcore::json::Value::Value(bool b)
   : mType(BooleanType)
{
   mValue.boo = b;
}

gcore::json::Value::Value(int num)
   : mType(NumberType)
{
   mValue.num = double(num);
}

gcore::json::Value::Value(float num)
   : mType(NumberType)
{
   mValue.num = double(num);
}

gcore::json::Value::Value(double num)
   : mType(NumberType)
{
   mValue.num = num;
}

gcore::json::Value::Value(gcore::String *str)
   : mType(str ? StringType : NullType)
{
   mValue.str = str;
}

gcore::json::Value::Value(const char *str)
   : mType(str ? StringType : NullType)
{
   mValue.str = (str ? new gcore::String(str) : 0);
}

gcore::json::Value::Value(const gcore::String &str)
   : mType(StringType)
{
   mValue.str = new gcore::String(str);
}

gcore::json::Value::Value(Object *obj)
   : mType(obj ? ObjectType : NullType)
{
   mValue.obj = obj;
}

gcore::json::Value::Value(const Object &obj)
   : mType(ObjectType)
{
   mValue.obj = new gcore::json::Object(obj);
}

gcore::json::Value::Value(Array *arr)
   : mType(arr ? ArrayType : NullType)
{
   mValue.arr = arr;
}

gcore::json::Value::Value(const Array &arr)
   : mType(ArrayType)
{
   mValue.arr = new gcore::json::Array(arr);
}

gcore::json::Value::Value(const gcore::json::Value &rhs)
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
      mValue.str = new gcore::String(*(rhs.mValue.str));
      break;
   case ObjectType:
      mValue.obj = new gcore::json::Object(*(rhs.mValue.obj));
      break;
   case ArrayType:
      mValue.arr = new gcore::json::Array(*(rhs.mValue.arr));
   default:
      break;
   }
}

gcore::json::Value::~Value()
{
   reset();
}

bool gcore::json::Value::toPropertyList(gcore::PropertyList &pl) const
{
   if (mType != ObjectType)
   {
      return false;
   }
   
   pl.create();
   
   return toPropertyList(pl, "");
}

bool gcore::json::Value::toPropertyList(gcore::PropertyList &pl, const gcore::String &cprop) const
{
   switch (mType)
   {
   case ObjectType:
      {
         gcore::String prop, bprop = cprop;
         
         if (cprop.length() > 0)
         {
            bprop += ".";
         }
         
         for (Object::const_iterator it=obegin(); it!=oend(); ++it)
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
         gcore::String prop, bprop = cprop + "[";
         size_t i = 0;
         
         for (Array::const_iterator it=abegin(); it!=aend(); ++it, ++i)
         {
            prop = bprop + gcore::String(i) + "]";
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

void gcore::json::Value::reset()
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

gcore::json::Value& gcore::json::Value::operator=(bool b)
{
   if (mType != BooleanType)
   {
      reset();
      mType = BooleanType;
   }
   
   mValue.boo = b;
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(int num)
{
   if (mType != NumberType)
   {
      reset();
      mType = NumberType;
   }
   
   mValue.num = double(num);
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(float num)
{
   if (mType != NumberType)
   {
      reset();
      mType = NumberType;
   }
   
   mValue.num = double(num);
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(double num)
{
   if (mType != NumberType)
   {
      reset();
      mType = NumberType;
   }
   
   mValue.num = num;
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(gcore::String *str)
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

gcore::json::Value& gcore::json::Value::operator=(const char *str)
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
         mValue.str = new gcore::String(str);
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

gcore::json::Value& gcore::json::Value::operator=(const gcore::String &str)
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
      mValue.str = new gcore::String(str);
   }
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(Object *obj)
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

gcore::json::Value& gcore::json::Value::operator=(const Object &obj)
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
      mValue.obj = new gcore::json::Object(obj);
   }
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(Array *arr)
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

gcore::json::Value& gcore::json::Value::operator=(const Array &arr)
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
      mValue.arr = new gcore::json::Array(arr);
   }
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(const gcore::json::Value &rhs)
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

gcore::json::Value::Type gcore::json::Value::type() const
{
   return mType;
}

gcore::json::Value::operator bool () const
{
   if (mType != BooleanType)
   {
      throw std::runtime_error("json::Value is not a boolean.");
   }
   return mValue.boo;
}

gcore::json::Value::operator int () const
{
   if (mType != NumberType)
   {
      throw std::runtime_error("json::Value is not a number.");
   }
   return int(mValue.num);
}

gcore::json::Value::operator float () const
{
   if (mType != NumberType)
   {
      throw std::runtime_error("json::Value is not a number.");
   }
   return float(mValue.num);
}

gcore::json::Value::operator double () const
{
   if (mType != NumberType)
   {
      throw std::runtime_error("json::Value is not a number.");
   }
   return mValue.num;
}

gcore::json::Value::operator const gcore::String& () const
{
   if (mType != StringType)
   {
      throw std::runtime_error("json::Value is not a string.");
   }
   return *(mValue.str);
}

gcore::json::Value::operator const char* () const
{
   if (mType != StringType)
   {
      throw std::runtime_error("json::Value is not a string.");
   }
   return mValue.str->c_str();
}

gcore::json::Value::operator const gcore::json::Object& () const
{
   if (mType != ObjectType)
   {
      throw std::runtime_error("json::Value is not an object.");
   }
   return *(mValue.obj);
}

gcore::json::Value::operator const gcore::json::Array& () const
{
   if (mType != ArrayType)
   {
      throw std::runtime_error("json::Value is not an array.");
   }
   return *(mValue.arr);
}

gcore::json::Value::operator gcore::String& ()
{
   if (mType != StringType)
   {
      throw std::runtime_error("json::Value is not a string.");
   }
   return *(mValue.str);
}

gcore::json::Value::operator gcore::json::Object& ()
{
   if (mType != ObjectType)
   {
      throw std::runtime_error("json::Value is not an object.");
   }
   return *(mValue.obj);
}

gcore::json::Value::operator gcore::json::Array& ()
{
   if (mType != ArrayType)
   {
      throw std::runtime_error("json::Value is not an array.");
   }
   return *(mValue.arr);
}

size_t gcore::json::Value::size() const
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

void gcore::json::Value::clear()
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

bool gcore::json::Value::insert(size_t pos, const gcore::json::Value &value)
{
   if (mType != ArrayType)
   {
      return false;
   }
   mValue.arr->insert(mValue.arr->begin() + pos, value);
   return true;
}

bool gcore::json::Value::insert(const gcore::String &key, const gcore::json::Value &value)
{
   if (mType != ObjectType)
   {
      return false;
   }
   (*(mValue.obj))[key] = value;
   return true;
}

bool gcore::json::Value::erase(size_t pos, size_t cnt)
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

bool gcore::json::Value::erase(const gcore::String &key)
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

static gcore::json::Array gsEmptyArray;
static gcore::json::Object gsEmptyObject;

gcore::json::ArrayConstIterator gcore::json::Value::abegin() const
{
   return (mType == ArrayType ? mValue.arr->begin() : gsEmptyArray.begin());
}

gcore::json::ArrayConstIterator gcore::json::Value::aend() const
{
   return (mType == ArrayType ? mValue.arr->end() : gsEmptyArray.end());
}

gcore::json::ArrayIterator gcore::json::Value::abegin()
{
   return (mType == ArrayType ? mValue.arr->begin() : gsEmptyArray.begin());
}

gcore::json::ArrayIterator gcore::json::Value::aend()
{
   return (mType == ArrayType ? mValue.arr->end() : gsEmptyArray.end());
}

const gcore::json::Value& gcore::json::Value::operator[](size_t idx) const
{
   if (mType != ArrayType)
   {
      throw std::runtime_error("json::Value is not an array.");
   }
   return mValue.arr->at(idx);
}

gcore::json::Value& gcore::json::Value::operator[](size_t idx)
{
   if (mType != ArrayType)
   {
      throw std::runtime_error("json::Value is not an array.");
   }
   return mValue.arr->at(idx);
}

gcore::json::ObjectConstIterator gcore::json::Value::obegin() const
{
   return (mType == ObjectType ? mValue.obj->begin() : gsEmptyObject.begin());
}

gcore::json::ObjectConstIterator gcore::json::Value::oend() const
{
   return (mType == ObjectType ? mValue.obj->end() : gsEmptyObject.end());
}

gcore::json::ObjectConstIterator gcore::json::Value::find(const gcore::String &name) const
{
   return (mType == ObjectType ? mValue.obj->find(name) : gsEmptyObject.end());
}

gcore::json::ObjectConstIterator gcore::json::Value::find(const char *name) const
{
   if (mType != ObjectType)
   {
      return gsEmptyObject.end();
   }
   gcore::String _name(name);
   return this->find(_name);
}

gcore::json::ObjectIterator gcore::json::Value::obegin()
{
   return (mType == ObjectType ? mValue.obj->begin() : gsEmptyObject.begin());
}

gcore::json::ObjectIterator gcore::json::Value::oend()
{
   return (mType == ObjectType ? mValue.obj->end() : gsEmptyObject.end());
}

gcore::json::ObjectIterator gcore::json::Value::find(const gcore::String &name)
{
   return (mType == ObjectType ? mValue.obj->find(name) : gsEmptyObject.end());
}

gcore::json::ObjectIterator gcore::json::Value::find(const char *name)
{
   if (mType != ObjectType)
   {
      return gsEmptyObject.end();
   }
   gcore::String _name(name);
   return this->find(_name);
}

const gcore::json::Value& gcore::json::Value::operator[](const gcore::String &name) const
{
   if (mType != ObjectType)
   {
      throw std::runtime_error("json::Value is not an object.");
   }
   Object::const_iterator it = mValue.obj->find(name);
   if (it == mValue.obj->end())
   {
      throw std::out_of_range("Invalid key \"" + name + "\".");
   }
   return it->second;
}

gcore::json::Value& gcore::json::Value::operator[](const gcore::String &name)
{
   if (mType != ObjectType)
   {
      throw std::runtime_error("json::Value is not an object.");
   }
   return (*mValue.obj)[name];
}

const gcore::json::Value& gcore::json::Value::operator[](const char *name) const
{
   gcore::String _name(name);
   return this->operator[](_name);
}

gcore::json::Value& gcore::json::Value::operator[](const char *name)
{
   gcore::String _name(name);
   return this->operator[](_name);
}

gcore::Status gcore::json::Value::write(const char *path) const
{
   if (!path)
   {
      return gcore::Status(false, "Invalid path.");
   }
   
   if (mType != ObjectType)
   {
      return gcore::Status(false, "Value is not an object.");
   }
   
   std::ofstream out(path);
   
   if (out.is_open())
   {
      write(out);
      return gcore::Status(true);
   }
   else
   {
      return gcore::Status(false, "Invalid file '%s'", path);
   }
}

void gcore::json::Value::write(std::ostream &os, const gcore::String indent, bool skipFirstIndent) const
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
      os << (skipFirstIndent ? "" : indent) << "\"" << mValue.str->c_str() << "\"";
      break;
   case ObjectType:
      {
         size_t i=0, n=mValue.obj->size();
         os << (skipFirstIndent ? "" : indent) << "{" << std::endl;
         for (Object::const_iterator it=mValue.obj->begin(); it!=mValue.obj->end(); ++it, ++i)
         {
            os << indent << "  \"" << it->first.c_str() << "\": ";
            it->second.write(os, indent + "  ", true);
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
            it->write(os, indent + "  ");
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

gcore::Status gcore::json::Value::read(const char *path)
{
   if (!path)
   {
      return gcore::Status(false, "Invalid path.");
   }
   
   std::ifstream in(path);
   
   if (!in.is_open())
   {
      reset();
      return gcore::Status(false, "Invalid file '%s'", path);
   }
   else
   {
      return read(in, true, 0);
   }
}

gcore::Status gcore::json::Value::read(std::istream &in)
{
   return read(in, false, 0);
}

gcore::Status gcore::json::Value::Parse(const char *path, gcore::json::Value::ParserCallbacks &callbacks)
{
   if (!path)
   {
      return Status(false, "Invalid path.");
   }
   
   json::Value val;
   
   std::ifstream in(path);
   
   if (in.is_open())
   {
      gcore::Status rv = val.read(in, true, &callbacks);
      val.reset();
      return rv;
   }
   else
   {
      return gcore::Status(false, "Invalid file '%s'", path);
   }
}

struct ParserStackItem
{
   gcore::json::Value::ParserState state;
   gcore::json::Value *value;
   size_t count;
   
   inline ParserStackItem(gcore::json::Value::ParserState s)
      : state(s)
      , value(0)
      , count(0)
   {
   }
   
   inline ParserStackItem(gcore::json::Value::ParserState s, gcore::json::Value *v)
      : state(s)
      , value(v)
      , count(0)
   {
   }
};

static gcore::Status Failed(gcore::json::Value *value, size_t line, size_t col, const char *fmt, ...)
{
   value->reset();
   
   if (!fmt)
   {
      return gcore::Status(false, " (line %u, column %u)", line, col);
   }
   else
   {
      char buffer[4096];
      va_list args;
      va_start(args, fmt);
      vsnprintf(buffer, 4095, fmt, args);
      va_end(args);
      return gcore::Status(false, "%s (line %u, column %u)", buffer, line, col);
   }
}

gcore::Status gcore::json::Value::read(std::istream &in, bool consumeAll, gcore::json::Value::ParserCallbacks *cb)
{
   static const char *sSpaces = " \t\r\n";
   
   reset();
   
   gcore::String remain, tmp;
   size_t p0, p1, lineno = 0, coloff = 0;
   
   std::vector<ParserStackItem> stack;
   ParserState state = Begin;
   gcore::String str = "";
   gcore::String key = "";
   bool readSep = true;
   bool hasSep = false;
   
   while (in.good())
   {
      if (remain.length() == 0)
      {
         // Note: getline discards the trailing '\n'
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
                  return Failed(this, lineno, coloff+p0, "Invalid parser state (read object)");
               }
               
               if (stack.back().count == 0)
               {
                  if (remain[p0] == ',')
                  {
                     return Failed(this, lineno, coloff+p0, "Unexpected ,");
                  }
                  
                  hasSep = false;
               }
               else
               {
                  if (remain[p0] != ',' && remain[p0] != '}')
                  {
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
               return Failed(this, lineno, coloff+p0, "Unexpected , before }");
            }
            
            if (stack.size() == 0)
            {
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
                  remain = remain.substr(p0 + 1);
                  
                  if (remain.strip().length() > 0)
                  {
                     return Failed(this, lineno, coloff+p0, "Unexpected characters after top level object's end");
                  }
                  else
                  {
                     return gcore::Status(true);
                  }
               }
            }
            else
            {
               state = stack.back().state;
               readSep = true;
               
               if (state != ReadObject && state != ReadArray)
               {
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
                  return Failed(this, lineno, coloff+p0, "Invalid parser state (read array)");
               }
               
               if (stack.back().count == 0)
               {
                  if (remain[p0] == ',')
                  {
                     return Failed(this, lineno, coloff+p0, "Unexpected ,");
                  }
                  
                  hasSep = false;
               }
               else
               {
                  if (remain[p0] != ',' && remain[p0] != ']')
                  {
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
               return Failed(this, lineno, coloff+p0, "Unexpected , before ]");
            }
            
            if (stack.size() == 0)
            {
               return Failed(this, lineno, coloff+p0, "Un-matched ]");
            }
            
            stack.pop_back();
            
            if (stack.size() == 0)
            {
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
                     // + 4 digits
                     if (p1 + 5 >= len || 
                         (str[p1 + 2] < '0' || str[p1 + 2] > '9') || 
                         (str[p1 + 3] < '0' || str[p1 + 3] > '9') || 
                         (str[p1 + 4] < '0' || str[p1 + 4] > '9') || 
                         (str[p1 + 5] < '0' || str[p1 + 5] > '9'))
                     {
                        return Failed(this, lineno, coloff+p1, "Expected 4 digits after \\u escape character");
                     }
                     p0 = p1 + 6;
                     break;
                  default:
                     return Failed(this, lineno, coloff+p1, "Unsupported escape character: \\%c", str[p1 + 1]);
                  }
                  
                  p1 = str.find('\\', p0);
               }
               else
               {
                  // trailing '\'
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
                  cb->objectKey(key.c_str());
               }
               
               p1 = remain.find_first_not_of(sSpaces);
               
               if (p1 == String::npos || remain[p1] != ':')
               {
                  return Failed(this, lineno, coloff+p1, "Expected : after string value");
               }
               
               remain = remain.substr(p1 + 1);
               
               state = ReadValue;
            }
            else
            {
               if (stack.size() == 0)
               {
                  return Failed(this, lineno, coloff, "Orphan string");
               }
               
               ParserStackItem &psi = stack.back();
               
               psi.count++;
               
               if (cb && cb->stringScalar)
               {
                  cb->stringScalar(str.c_str());
               }
               
               if (psi.value)
               {
                  if (psi.value->type() == ObjectType)
                  {
                     if (key.length() == 0)
                     {
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
               return Failed(this, lineno, coloff+p0, "Orphan value");
            }
            
            ParserStackItem &psi = stack.back();
            
            if (psi.state == ReadObject)
            {
               if (key.length() == 0)
               {
                  return Failed(this, lineno, coloff+p0, "Undefined or empty object member name");
               }
            }
            else if (psi.state != ReadArray)
            {
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
                  if (!strncmp(remain.c_str(), "null", 4))
                  {
                     if (cb && cb->nullScalar)
                     {
                        cb->nullScalar();
                     }
                     remain = remain.substr(p0 + 4);
                     coloff += p0 + 4;
                  }
                  else if (!strncmp(remain.c_str(), "true", 4))
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
                  else if (!strncmp(remain.c_str(), "false", 5))
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
                     gcore::String numstr;
                     
                     p1 = remain.find_first_of(sSpaces, p0);
                     
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
         
         p0 = remain.find_first_not_of(sSpaces);
         
         if (p0 != String::npos)
         {
            return Failed(this, lineno, coloff+p0, "Content after top level object");
         }
         
      default:
         break;
      }
   }
   
   return Status(true);
}

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
