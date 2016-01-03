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

gcore::json::Exception::Exception(const gcore::String &msg)
   : std::exception()
   , mMsg(msg)
{
}

gcore::json::Exception::Exception(const char *fmt, ...)
{
   char buffer[1024];
   va_list vl;
   va_start(vl, fmt);  
   vsprintf(buffer, fmt, vl);
   va_end(vl);
   mMsg = buffer;
}

gcore::json::Exception::~Exception() throw()
{
}

const char* gcore::json::Exception::what() const throw()
{
   return mMsg.c_str();
}

// ---

gcore::json::ParserError::ParserError(size_t _line, size_t _col, const gcore::String &msg)
   : gcore::json::Exception(msg)
   , mLine(_line)
   , mCol(_col)
{
   mMsg += " (line " + gcore::String(mLine) + ", column " + gcore::String(mCol) + ")";
}

gcore::json::ParserError::ParserError(size_t _line, size_t _col, const char *fmt, ...)
   : gcore::json::Exception("")
   , mLine(_line)
   , mCol(_col)
{
   char buffer[1024];
   va_list vl;
   va_start(vl, fmt);  
   vsprintf(buffer, fmt, vl);
   va_end(vl);
   
   mMsg = gcore::String(buffer) + " (line " + gcore::String(mLine) + ", column " + gcore::String(mCol) + ")";
}

gcore::json::ParserError::~ParserError() throw()
{
}

// ---

gcore::json::TypeError::TypeError(const gcore::String &msg)
   : gcore::json::Exception(msg)
{
}

gcore::json::TypeError::TypeError(const char *fmt, ...)
   : gcore::json::Exception("")
{
   char buffer[1024];
   va_list vl;
   va_start(vl, fmt);  
   vsprintf(buffer, fmt, vl);
   va_end(vl);
   
   mMsg = buffer;
}

gcore::json::TypeError::~TypeError() throw()
{
}

// ---

gcore::json::MemberError::MemberError(const gcore::String &name)
   : Exception("Invalid object member \"" + name + "\"")
   , mName(name)
{
}

gcore::json::MemberError::~MemberError() throw()
{
}

// ---

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
   try
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
               it->second.toPropertyList(pl, prop);
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
               it->toPropertyList(pl, prop);
            }
         }
         break;
      
      case BooleanType:
         pl.setBoolean(cprop, mValue.boo);
         break;
      
      case NumberType:
         pl.setReal(cprop, mValue.num);
         break;
      
      case StringType:
         pl.setString(cprop, *(mValue.str));
         break;
      
      case NullType:
         // Ignore key
      default:
         break;
      }
      
      return true;
   }
   catch (gcore::plist::Exception &)
   {
      return false;
   }
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
      throw gcore::json::TypeError("Value is not a boolean");
   }
   return mValue.boo;
}

gcore::json::Value::operator int () const
{
   if (mType != NumberType)
   {
      throw gcore::json::TypeError("Value is not a number");
   }
   return int(mValue.num);
}

gcore::json::Value::operator float () const
{
   if (mType != NumberType)
   {
      throw gcore::json::TypeError("Value is not a number");
   }
   return float(mValue.num);
}

gcore::json::Value::operator double () const
{
   if (mType != NumberType)
   {
      throw gcore::json::TypeError("Value is not a number");
   }
   return mValue.num;
}

gcore::json::Value::operator const gcore::String& () const
{
   if (mType != StringType)
   {
      throw gcore::json::TypeError("Value is not a string");
   }
   return *(mValue.str);
}

gcore::json::Value::operator const char* () const
{
   if (mType != StringType)
   {
      throw gcore::json::TypeError("Value is not a string");
   }
   return mValue.str->c_str();
}

gcore::json::Value::operator const Object& () const
{
   if (mType != ObjectType)
   {
      throw gcore::json::TypeError("Value is not an object");
   }
   return *(mValue.obj);
}

gcore::json::Value::operator const Array& () const
{
   if (mType != ArrayType)
   {
      throw gcore::json::TypeError("Value is not an array");
   }
   return *(mValue.arr);
}

gcore::json::Value::operator gcore::String& ()
{
   if (mType != StringType)
   {
      throw gcore::json::TypeError("Value is not a string");
   }
   return *(mValue.str);
}

gcore::json::Value::operator Object& ()
{
   if (mType != ObjectType)
   {
      throw gcore::json::TypeError("Value is not an object");
   }
   return *(mValue.obj);
}

gcore::json::Value::operator Array& ()
{
   if (mType != ArrayType)
   {
      throw gcore::json::TypeError("Value is not an array");
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

gcore::json::ArrayConstIterator gcore::json::Value::abegin() const
{
   if (mType != ArrayType)
   {
      throw TypeError("Value is not an array");
   }
   return mValue.arr->begin();
}

gcore::json::ArrayConstIterator gcore::json::Value::aend() const
{
   if (mType != ArrayType)
   {
      throw TypeError("Value is not an array");
   }
   return mValue.arr->end();
}

gcore::json::ArrayIterator gcore::json::Value::abegin()
{
   if (mType != ArrayType)
   {
      throw TypeError("Value is not an array");
   }
   return mValue.arr->begin();
}

gcore::json::ArrayIterator gcore::json::Value::aend()
{
   if (mType != ArrayType)
   {
      throw TypeError("Value is not an array");
   }
   return mValue.arr->end();
}

const gcore::json::Value& gcore::json::Value::operator[](size_t idx) const
{
   if (mType != ArrayType)
   {
      throw TypeError("Value is not an array");
   }
   return mValue.arr->at(idx);
}

gcore::json::Value& gcore::json::Value::operator[](size_t idx)
{
   if (mType != ArrayType)
   {
      throw TypeError("Value is not an array");
   }
   return mValue.arr->at(idx);
}

void gcore::json::Value::insert(size_t pos, const Value &value)
{
   if (mType != ArrayType)
   {
      throw TypeError("Value is not an array");
   }
   mValue.arr->insert(mValue.arr->begin() + pos, value);
}

void gcore::json::Value::erase(size_t pos, size_t cnt)
{
   if (mType != ArrayType)
   {
      throw TypeError("Value is not an array");
   }
   size_t n = mValue.arr->size();
   if (pos >= n)
   {
      return;
   }
   if (pos + cnt > n)
   {
      cnt = n - pos;
   }
   Array::iterator first = mValue.arr->begin() + pos;
   Array::iterator last = first + cnt;
   mValue.arr->erase(first, last);
}

gcore::json::ObjectConstIterator gcore::json::Value::obegin() const
{
   if (mType != ObjectType)
   {
      throw TypeError("Value is not an object");
   }
   return mValue.obj->begin();
}

gcore::json::ObjectConstIterator gcore::json::Value::oend() const
{
   if (mType != ObjectType)
   {
      throw TypeError("Value is not an object");
   }
   return mValue.obj->end();
}

gcore::json::ObjectConstIterator gcore::json::Value::find(const gcore::String &name) const
{
   if (mType != ObjectType)
   {
      throw TypeError("Value is not an object");
   }
   return mValue.obj->find(name);
}

gcore::json::ObjectConstIterator gcore::json::Value::find(const char *name) const
{
   gcore::String _name(name);
   return this->find(_name);
}

gcore::json::ObjectIterator gcore::json::Value::obegin()
{
   if (mType != ObjectType)
   {
      throw TypeError("Value is not an object");
   }
   return mValue.obj->begin();
}

gcore::json::ObjectIterator gcore::json::Value::oend()
{
   if (mType != ObjectType)
   {
      throw TypeError("Value is not an object");
   }
   return mValue.obj->end();
}

gcore::json::ObjectIterator gcore::json::Value::find(const gcore::String &name)
{
   if (mType != ObjectType)
   {
      throw TypeError("Value is not an object");
   }
   return mValue.obj->find(name);
}

gcore::json::ObjectIterator gcore::json::Value::find(const char *name)
{
   gcore::String _name(name);
   return this->find(_name);
}

const gcore::json::Value& gcore::json::Value::operator[](const gcore::String &name) const
{
   if (mType != ObjectType)
   {
      throw TypeError("Value is not an object");
   }
   Object::const_iterator it = mValue.obj->find(name);
   if (it == mValue.obj->end())
   {
      throw MemberError(name);
   }
   return it->second;
}

gcore::json::Value& gcore::json::Value::operator[](const gcore::String &name)
{
   if (mType != ObjectType)
   {
      throw TypeError("Value is not an object");
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

bool gcore::json::Value::write(const char *path) const
{
   if (mType != ObjectType)
   {
      return false;
   }
   
   std::ofstream out(path);
   
   if (out.is_open())
   {
      write(out);
      return true;
   }
   else
   {
      return false;
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

void gcore::json::Value::read(const char *path)
{
   std::ifstream in(path);
   
   if (!in.is_open())
   {
      reset();
   }
   else
   {
      read(in, true, 0);
   }
}

void gcore::json::Value::read(std::istream &in)
{
   read(in, false, 0);
}

void gcore::json::Value::Parse(const char *path, ParserCallbacks *callbacks)
{
   if (!callbacks)
   {
      return;
   }
   
   json::Value val;
   
   std::ifstream in(path);
   
   if (in.is_open())
   {
      val.read(in, true, callbacks);
      val.reset();
   }
}

void gcore::json::Value::read(std::istream &in, bool consumeAll, gcore::json::Value::ParserCallbacks *cb)
{
   static const char *sSpaces = " \t\r\n";
   
   reset();
   
   // only keep previous line?
   gcore::String remain;
   size_t p0, p1, lineno = 0, coloff = 0;
   
   std::vector<Value*> valueStack;
   Value *curValue = 0;
   gcore::String str = "";
   gcore::String key = "";
   ParserState state = Begin;
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
         
         if (p0 == std::string::npos)
         {
            remain = "";
         }
         else
         {
            if (remain[p0] != '{')
            {
               reset();
               throw ParserError(lineno, coloff+p0, "Expect object at top level");
            }
            else
            {
               mType = ObjectType;
               mValue.obj = new Object();
               
               curValue = this;
               
               if (cb && cb->objectBegin)
               {
                  cb->objectBegin();
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
            if (p0 != std::string::npos)
            {
               if (curValue->size() == 0)
               {
                  if (remain[p0] == ',')
                  {
                     reset();
                     throw ParserError(lineno, coloff+p0, "Unexpected ,");
                  }
                  
                  hasSep = false;
               }
               else
               {
                  if (remain[p0] != ',' && remain[p0] != '}')
                  {
                     reset();
                     throw ParserError(lineno, coloff+p0, "Expected , or }");
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
         
         if (p0 == std::string::npos)
         {
            remain = "";
         }
         else if (remain[p0] == '}')
         {
            if (hasSep)
            {
               reset();
               throw ParserError(lineno, coloff+p0, "Unexpected , before }");
            }
            
            if (cb && cb->objectEnd)
            {
               cb->objectEnd();
            }
            
            if (valueStack.size() == 0)
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
                     reset();
                     throw ParserError(lineno, coloff+p0, "Unexpected characters after top level object's end");
                  }
                  else
                  {
                     return;
                  }
               }
            }
            else
            {
               curValue = valueStack.back();
               valueStack.pop_back();
               
               if (curValue->type() == ObjectType)
               {
                  state = ReadObject;
                  readSep = true;
               }
               else if (curValue->type() == ArrayType)
               {
                  state = ReadArray;
                  readSep = true;
               }
               else
               {
                  reset();
                  throw ParserError(lineno, coloff+p0, "Parent value must be either an object or an array");
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
            reset();
            throw ParserError(lineno, coloff+p0, "Expect string value");
         }
         
         break;
      
      case ReadArray:
         #ifdef _DEBUG
         std::cout << "Parse|ReadArray (remain = '" << remain << "')" << std::endl;
         #endif
         
         p0 = remain.find_first_not_of(sSpaces);
         
         if (readSep)
         {
            if (p0 != std::string::npos)
            {
               if (curValue->size() == 0)
               {
                  if (remain[p0] == ',')
                  {
                     reset();
                     throw ParserError(lineno, coloff+p0, "Unexpected ,");
                  }
                  
                  hasSep = false;
               }
               else
               {
                  if (remain[p0] != ',' && remain[p0] != ']')
                  {
                     reset();
                     throw ParserError(lineno, coloff+p0, "Expected , or ]");
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
         
         if (p0 == std::string::npos)
         {
            remain = "";
         }
         else if (remain[p0] == ']')
         {
            if (hasSep)
            {
               reset();
               throw ParserError(lineno, coloff+p0, "Unexpected , before ]");
            }
            
            if (valueStack.size() == 0)
            {
               reset();
               throw ParserError(lineno, coloff+p0, "Orphan array value");
            }
            else
            {
               if (cb && cb->arrayEnd)
               {
                  cb->arrayEnd();
               }
               
               curValue = valueStack.back();
               valueStack.pop_back();
               
               if (curValue->type() == ObjectType)
               {
                  state = ReadObject;
                  readSep = true;
               }
               else if (curValue->type() == ArrayType)
               {
                  state = ReadArray;
                  readSep = true;
               }
               else
               {
                  reset();
                  throw ParserError(lineno, coloff+p0, "Parent value must be either an object or an array");
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
         
         while (p1 != std::string::npos)
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
         
         if (p1 == std::string::npos)
         {
            // couldn't find closing "
            str += remain + "\n";
            remain = "";
         }
         else
         {
            if (state == ReadObjectKey)
            {
               key = str;
               #ifdef _DEBUG
               std::cout << "Parse|ReadObjectKey -> " << key << std::endl;
               #endif
               
               if (cb && cb->objectKey)
               {
                  cb->objectKey(key.c_str());
               }
               
               p1 = remain.find_first_not_of(sSpaces);
               
               if (p1 == std::string::npos || remain[p1] != ':')
               {
                  reset();
                  throw ParserError(lineno, coloff+p1, "Expected : after string value");
               }
               
               remain = remain.substr(p1 + 1);
               
               state = ReadValue;
            }
            else
            {
               *curValue = str;
               
               if (valueStack.size() == 0)
               {
                  reset();
                  throw ParserError(lineno, coloff, "Orphan string");
               }
               
               if (cb && cb->stringScalar)
               {
                  cb->stringScalar(str.c_str());
               }
               
               curValue = valueStack.back();
               valueStack.pop_back();
               
               if (curValue->type() == ObjectType)
               {
                  state = ReadObject;
                  readSep = true;
               }
               else if (curValue->type() == ArrayType)
               {
                  state = ReadArray;
                  readSep = true;
               }
               else
               {
                  reset();
                  throw ParserError(lineno, coloff, "Parent value must be either an object or an array");
               }
            }
         }
         
         break;
      
      case ReadValue:
         #ifdef _DEBUG
         std::cout << "Parse|ReadValue (remain = '" << remain << "')" << std::endl;
         #endif
         
         p0 = remain.find_first_not_of(sSpaces);
         
         if (p0 == std::string::npos)
         {
            remain = "";
         }
         else
         {
            valueStack.push_back(curValue);
            
            if (curValue->type() == ObjectType)
            {
               if (key.length() == 0)
               {
                  reset();
                  throw ParserError(lineno, coloff+p0, "Undefined or empty object member name");
               }
               
               curValue = &((*curValue)[key]);
               
               // reset key
               key = "";
            }
            else if (curValue->type() == ArrayType)
            {
               size_t idx = curValue->size();
               curValue->insert(idx, Value());
               
               curValue = &((*curValue)[idx]);
            }
            else
            {
               reset();
               throw ParserError(lineno, coloff+p0, "Parent value must be either an object or an array");
            }
            
            if (remain[p0] == '{')
            {
               *curValue = Object();
               if (cb && cb->objectBegin)
               {
                  cb->objectBegin();
               }
               state = ReadObject;
               readSep = true;
               remain = remain.substr(p0 + 1);
               coloff += p0 + 1;
            }
            else if (remain[p0] == '[')
            {
               *curValue = Array();
               if (cb && cb->arrayBegin)
               {
                  cb->arrayBegin();
               }
               state = ReadArray;
               readSep = true;
               remain = remain.substr(p0 + 1);
               coloff += p0 + 1;
            }
            else if (remain[p0] == '"')
            {
               *curValue = "";
               state = ReadString;
               str = "";
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
                  *curValue = true;
                  remain = remain.substr(p0 + 4);
                  coloff += p0 + 4;
               }
               else if (!strncmp(remain.c_str(), "false", 5))
               {
                  if (cb && cb->booleanScalar)
                  {
                     cb->booleanScalar(false);
                  }
                  *curValue = false;
                  remain = remain.substr(p0 + 5);
                  coloff += p0 + 5;
               }
               else
               {
                  // must be a number
                  gcore::String numstr;
                  
                  p1 = remain.find_first_of(sSpaces);
                  
                  if (p1 == std::string::npos)
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
                     reset();
                     throw ParserError(lineno, coloff, "Expected number value");
                  }
                  
                  if (cb && cb->numberScalar)
                  {
                     cb->numberScalar(val);
                  }
                  
                  *curValue = val;
               }
               
               // at this point we know stack is not empty
               // and that last element is either an object or an array
               curValue = valueStack.back();
               valueStack.pop_back();
               
               if (curValue->type() == ObjectType)
               {
                  state = ReadObject;
                  readSep = true;
               }
               else
               {
                  state = ReadArray;
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
         
         if (p0 != std::string::npos)
         {
            reset();
            throw ParserError(lineno, coloff+p0, "Content after top level object");
         }
         
      default:
         break;
      }
   }
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
