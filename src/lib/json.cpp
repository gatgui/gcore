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
   , mBool(false)
   , mNum(0.0)
   , mStr("")
   , mObj(0)
   , mArr(0)
{
}

gcore::json::Value::Value(Type t)
   : mType(t)
   , mBool(false)
   , mNum(0.0)
   , mStr("")
   , mObj(t == ObjectType ? new gcore::json::Object() : 0)
   , mArr(t == ArrayType ? new gcore::json::Array() : 0)
{
}

gcore::json::Value::Value(bool b)
   : mType(BooleanType)
   , mBool(b)
   , mNum(0.0)
   , mStr("")
   , mObj(0)
   , mArr(0)
{
}

gcore::json::Value::Value(int num)
   : mType(NumberType)
   , mBool(false)
   , mNum(num)
   , mStr("")
   , mObj(0)
   , mArr(0)
{
}

gcore::json::Value::Value(float num)
   : mType(NumberType)
   , mBool(false)
   , mNum(num)
   , mStr("")
   , mObj(0)
   , mArr(0)
{
}

gcore::json::Value::Value(double num)
   : mType(NumberType)
   , mBool(false)
   , mNum(num)
   , mStr("")
   , mObj(0)
   , mArr(0)
{
}

gcore::json::Value::Value(const char *str)
   : mType(str ? StringType : NullType)
   , mBool(false)
   , mNum(0.0)
   , mStr(str ? str : "")
   , mObj(0)
   , mArr(0)
{
}

gcore::json::Value::Value(const gcore::String &str)
   : mType(StringType)
   , mBool(false)
   , mNum(0.0)
   , mStr(str)
   , mObj(0)
   , mArr(0)
{
}

gcore::json::Value::Value(Object *obj)
   : mType(obj ? ObjectType : NullType)
   , mBool(false)
   , mNum(0.0)
   , mStr("")
   , mObj(obj)
   , mArr(0)
{
}

gcore::json::Value::Value(const Object &obj)
   : mType(ObjectType)
   , mBool(false)
   , mNum(0.0)
   , mStr("")
   , mObj(new gcore::json::Object(obj))
   , mArr(0)
{
}

gcore::json::Value::Value(Array *arr)
   : mType(arr ? ArrayType : NullType)
   , mBool(false)
   , mNum(0.0)
   , mStr("")
   , mObj(0)
   , mArr(arr)
{
}

gcore::json::Value::Value(const Array &arr)
   : mType(ArrayType)
   , mBool(false)
   , mNum(0.0)
   , mStr("")
   , mObj(0)
   , mArr(new gcore::json::Array(arr))
{
}

gcore::json::Value::Value(const gcore::json::Value &rhs)
   : mType(rhs.mType)
   , mBool(rhs.mBool)
   , mNum(rhs.mNum)
   , mStr(rhs.mStr)
   , mObj(0)
   , mArr(0)
{
   if (rhs.mObj)
   {
      mObj = new gcore::json::Object(*(rhs.mObj));
   }
   if (rhs.mArr)
   {
      mArr = new gcore::json::Array(*(rhs.mArr));
   }
}

gcore::json::Value::~Value()
{
   reset();
}

void gcore::json::Value::reset()
{
   if (mObj)
   {
      delete mObj;
      mObj = 0;
   }
   
   if (mArr)
   {
      delete mArr;
      mArr = 0;
   }
   
   mBool = false;
   mNum = 0.0;
   mStr = "";
   mType = NullType;
}

gcore::json::Value& gcore::json::Value::operator=(bool b)
{
   if (mType != BooleanType)
   {
      reset();
      mType = BooleanType;
   }
   
   mBool = b;
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(int num)
{
   if (mType != NumberType)
   {
      reset();
      mType = NumberType;
   }
   
   mNum = num;
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(float num)
{
   if (mType != NumberType)
   {
      reset();
      mType = NumberType;
   }
   
   mNum = num;
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(double num)
{
   if (mType != NumberType)
   {
      reset();
      mType = NumberType;
   }
   
   mNum = num;
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(const char *str)
{
   if (mType != StringType)
   {
      reset();
      mType = StringType;
   }
   
   if (str)
   {
      mStr = str;
   }
   else
   {
      mStr = "";
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
   }
   
   mStr = str;
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(Object *obj)
{
   if (mType != ObjectType)
   {
      reset();
      mType = ObjectType;
   }
   
   if (obj)
   {
      if (mObj && mObj != obj)
      {
         delete mObj;
      }
      mObj = obj;
   }
   else
   {
      if (mObj)
      {
         delete mObj;
         mObj = 0;
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
   }
   
   if (mObj)
   {
      *mObj = obj;
   }
   else
   {
      mObj = new gcore::json::Object(obj);
   }
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(Array *arr)
{
   if (mType != ArrayType)
   {
      reset();
      mType = ArrayType;
   }
   
   if (arr)
   {
      if (mArr && mArr != arr)
      {
         delete mArr;
      }
      mArr = arr;
   }
   else
   {
      if (mArr)
      {
         delete mArr;
         mArr = 0;
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
   }
   
   if (mArr)
   {
      *mArr = arr;
   }
   else
   {
      mArr = new gcore::json::Array(arr);
   }
   
   return *this;
}

gcore::json::Value& gcore::json::Value::operator=(const gcore::json::Value &rhs)
{
   if (this != &rhs)
   {
      if (mType != rhs.mType)
      {
         reset();
         mType = rhs.mType;
      }
      
      switch (mType)
      {
      case BooleanType:
         mBool = rhs.mBool;
         break;
      case NumberType:
         mNum = rhs.mNum;
         break;
      case StringType:
         mStr = rhs.mStr;
         break;
      case ObjectType:
         if (mObj)
         {
            *mObj = *(rhs.mObj);
         }
         else
         {
            mObj = new gcore::json::Object(*(rhs.mObj));
         }
         break;
      case ArrayType:
         if (mArr)
         {
            *mArr = *(rhs.mArr);
         }
         else
         {
            mArr = new gcore::json::Array(*(rhs.mArr));
         }
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
   return mBool;
}

gcore::json::Value::operator int () const
{
   if (mType != NumberType)
   {
      throw gcore::json::TypeError("Value is not a number");
   }
   return int(mNum);
}

gcore::json::Value::operator float () const
{
   if (mType != NumberType)
   {
      throw gcore::json::TypeError("Value is not a number");
   }
   return float(mNum);
}

gcore::json::Value::operator double () const
{
   if (mType != NumberType)
   {
      throw gcore::json::TypeError("Value is not a number");
   }
   return mNum;
}

gcore::json::Value::operator const gcore::String& () const
{
   if (mType != StringType)
   {
      throw gcore::json::TypeError("Value is not a string");
   }
   return mStr;
}

gcore::json::Value::operator const char* () const
{
   if (mType != StringType)
   {
      throw gcore::json::TypeError("Value is not a string");
   }
   return mStr.c_str();
}

gcore::json::Value::operator const Object& () const
{
   if (mType != ObjectType)
   {
      throw gcore::json::TypeError("Value is not an object");
   }
   return *mObj;
}

gcore::json::Value::operator const Array& () const
{
   if (mType != ArrayType)
   {
      throw gcore::json::TypeError("Value is not an array");
   }
   return *mArr;
}

gcore::json::Value::operator Object& ()
{
   if (mType != ObjectType)
   {
      throw gcore::json::TypeError("Value is not an object");
   }
   return *mObj;
}

gcore::json::Value::operator Array& ()
{
   if (mType != ArrayType)
   {
      throw gcore::json::TypeError("Value is not an array");
   }
   return *mArr;
}

size_t gcore::json::Value::size() const
{
   if (mType == ArrayType)
   {
      return mArr->size();
   }
   else if (mType == ObjectType)
   {
      return mObj->size();
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
      mArr->clear();
   }
   else if (mType == ObjectType)
   {
      mObj->clear();
   }
}

gcore::json::ArrayConstIterator gcore::json::Value::abegin() const
{
   if (mType != ArrayType)
   {
      throw TypeError("Value is not an array");
   }
   return mArr->begin();
}

gcore::json::ArrayConstIterator gcore::json::Value::aend() const
{
   if (mType != ArrayType)
   {
      throw TypeError("Value is not an array");
   }
   return mArr->end();
}

gcore::json::ArrayIterator gcore::json::Value::abegin()
{
   if (mType != ArrayType)
   {
      throw TypeError("Value is not an array");
   }
   return mArr->begin();
}

gcore::json::ArrayIterator gcore::json::Value::aend()
{
   if (mType != ArrayType)
   {
      throw TypeError("Value is not an array");
   }
   return mArr->end();
}

const gcore::json::Value& gcore::json::Value::operator[](size_t idx) const
{
   if (mType != ArrayType)
   {
      throw TypeError("Value is not an array");
   }
   return mArr->at(idx);
}

gcore::json::Value& gcore::json::Value::operator[](size_t idx)
{
   if (mType != ArrayType)
   {
      throw TypeError("Value is not an array");
   }
   return mArr->at(idx);
}

void gcore::json::Value::insert(size_t pos, const Value &value)
{
   if (mType != ArrayType)
   {
      throw TypeError("Value is not an array");
   }
   mArr->insert(mArr->begin() + pos, value);
}

void gcore::json::Value::erase(size_t pos, size_t cnt)
{
   if (mType != ArrayType)
   {
      throw TypeError("Value is not an array");
   }
   size_t n = mArr->size();
   if (pos >= n)
   {
      return;
   }
   if (pos + cnt > n)
   {
      cnt = n - pos;
   }
   Array::iterator first = mArr->begin() + pos;
   Array::iterator last = first + cnt;
   mArr->erase(first, last);
}

gcore::json::ObjectConstIterator gcore::json::Value::obegin() const
{
   if (mType != ObjectType)
   {
      throw TypeError("Value is not an object");
   }
   return mObj->begin();
}

gcore::json::ObjectConstIterator gcore::json::Value::oend() const
{
   if (mType != ObjectType)
   {
      throw TypeError("Value is not an object");
   }
   return mObj->end();
}

gcore::json::ObjectConstIterator gcore::json::Value::find(const gcore::String &name) const
{
   if (mType != ObjectType)
   {
      throw TypeError("Value is not an object");
   }
   return mObj->find(name);
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
   return mObj->begin();
}

gcore::json::ObjectIterator gcore::json::Value::oend()
{
   if (mType != ObjectType)
   {
      throw TypeError("Value is not an object");
   }
   return mObj->end();
}

gcore::json::ObjectIterator gcore::json::Value::find(const gcore::String &name)
{
   if (mType != ObjectType)
   {
      throw TypeError("Value is not an object");
   }
   return mObj->find(name);
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
   Object::const_iterator it = mObj->find(name);
   if (it == mObj->end())
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
   return (*mObj)[name];
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
      os << (skipFirstIndent ? "" : indent) << (mBool ? "true" : "false");
      break;
   case NumberType:
      os << (skipFirstIndent ? "" : indent) << mNum;
      break;
   case StringType:
      os << (skipFirstIndent ? "" : indent) << "\"" << mStr << "\"";
      break;
   case ObjectType:
      {
         size_t i=0, n=mObj->size();
         os << (skipFirstIndent ? "" : indent) << "{" << std::endl;
         for (Object::const_iterator it=mObj->begin(); it!=mObj->end(); ++it, ++i)
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
         size_t i=0, n=mArr->size();
         os << (skipFirstIndent ? "" : indent) << "[" << std::endl;
         for (Array::const_iterator it=mArr->begin(); it!=mArr->end(); ++it, ++i)
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
      read(in, true);
   }
}

void gcore::json::Value::read(std::istream &in, bool consumeAll)
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
               mObj = new Object();
               
               curValue = this;
               
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
               state = ReadObject;
               readSep = true;
               remain = remain.substr(p0 + 1);
               coloff += p0 + 1;
            }
            else if (remain[p0] == '[')
            {
               *curValue = Array();
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
                  remain = remain.substr(p0 + 4);
                  coloff += p0 + 4;
               }
               else if (!strncmp(remain.c_str(), "true", 4))
               {
                  *curValue = true;
                  remain = remain.substr(p0 + 4);
                  coloff += p0 + 4;
               }
               else if (!strncmp(remain.c_str(), "false", 5))
               {
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
