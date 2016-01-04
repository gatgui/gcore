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

#ifndef __gcore_json_h_
#define __gcore_json_h_

#include <gcore/string.h>
#include <gcore/functor.h>

namespace gcore
{
   class PropertyList;
   
   namespace json
   {
      class GCORE_API Exception : public std::exception
      {
      public:
         explicit Exception(const gcore::String &msg);
         explicit Exception(const char *fmt, ...);
         virtual ~Exception() throw();
          
         virtual const char* what() const throw();
      
      protected:
         gcore::String mMsg;
      };
      
      class GCORE_API ParserError : public Exception
      {
      public:
         explicit ParserError(size_t line, size_t col, const gcore::String &msg);
         explicit ParserError(size_t line, size_t col, const char *fmt, ...);
         virtual ~ParserError() throw();
         
         inline size_t line() const { return mLine; }
         inline size_t column() const { return mCol; }
         
      protected:
         size_t mLine;
         size_t mCol;
      };
      
      class GCORE_API TypeError : public Exception
      {
      public:
         explicit TypeError(const gcore::String &msg);
         explicit TypeError(const char *fmt, ...);
         virtual ~TypeError() throw();
      };
      
      class GCORE_API MemberError : public Exception
      {
      public:
         explicit MemberError(const gcore::String &name);
         virtual ~MemberError() throw();
         
         inline const char* name() const { return mName.c_str(); }
         
      protected:
         gcore::String mName;
      };
      
      class GCORE_API Value
      {
      public:
         enum Type
         {
            NullType = 0,
            BooleanType,
            NumberType,
            StringType,
            ArrayType,
            ObjectType
         };
         
         typedef std::map<gcore::String, Value> Object;
         typedef std::deque<Value> Array;
         
         // Cannot use Object::iterator, Object::const_iterator, Array::iterator,
         //   Array::const_iterator directly in Value class methods as Value class
         //   is yet begin defined
         // Work around by declaring a template class for iterators so that type
         //   instancing is postponed
         
         template <class T>
         class iterator : public T::iterator
         {
         public:
            iterator(const typename T::iterator &rhs)
               : T::iterator(rhs)
            {
            }
            iterator& operator=(const typename T::iterator &rhs)
            {
               T::iterator::operator=(rhs);
               return *this;
            }
         };
         
         template <class T>
         class const_iterator : public T::const_iterator
         {
         public:
            const_iterator(const typename T::iterator &rhs)
               : T::const_iterator(rhs)
            {
            }
            const_iterator(const typename T::const_iterator &rhs)
               : T::const_iterator(rhs)
            {
            }
            const_iterator& operator=(const typename T::const_iterator &rhs)
            {
               T::const_iterator::operator=(rhs);
               return *this;
            }
         };
         
      public:
         
         Value();
         Value(Type t);
         Value(bool b);
         Value(int num);
         Value(float num);
         Value(double num);
         Value(gcore::String *str); // steals ownership
         Value(const char *str);
         Value(const gcore::String &str);
         Value(Object *obj); // steals ownership
         Value(const Object &obj);
         Value(Array *arr); // steals ownership
         Value(const Array &arr);
         Value(const Value &rhs);
         ~Value();
         
         bool toPropertyList(gcore::PropertyList &pl) const;
         
         Value& operator=(const Value &rhs);
         Value& operator=(bool b);
         Value& operator=(int num);
         Value& operator=(float num);
         Value& operator=(double num);
         Value& operator=(gcore::String *str); // steals ownership
         Value& operator=(const char *str);
         Value& operator=(const gcore::String &str);
         Value& operator=(Object *obj); // steals ownership
         Value& operator=(const Object &obj);
         Value& operator=(Array *arr); // steals ownership
         Value& operator=(const Array &arr);
         
         Type type() const;
         
         // All the cast operators may throw a TypeError exception
         operator bool () const;
         operator int () const;
         operator float () const;
         operator double () const;
         operator const gcore::String& () const;
         operator const char* () const;
         operator const Object& () const;
         operator const Array& () const;
         operator gcore::String& ();
         operator Object& ();
         operator Array& ();
         
         void reset();
         
         // Read methods may throw ParserError exception
         void read(const char *path);
         void read(std::istream &is);
          
         bool write(const char *path) const;
         void write(std::ostream &os, const gcore::String indent="", bool skipFirstIndent=false) const;
         
         // The remaining methods are shortcuts for Array and Object type values
         // size()  => ((const Array&)value).size()
         //            ((const Object&)value).size()
         // clear() => ((Array&)value).clear()
         //            ((Object&)value).clear()
         // ...
         
         // ArrayType or ObjectType.
         // - size returns 0 for any other type
         // - clear does nothing for ant other type
         size_t size() const;
         void clear();
         
         // ArrayType only, may throw TypeError exception
         iterator<Array> abegin();
         const_iterator<Array> abegin() const;
         iterator<Array> aend();
         const_iterator<Array> aend() const;
         const Value& operator[](size_t idx) const;
         Value& operator[](size_t idx);
         void insert(size_t pos, const Value &value);
         void erase(size_t pos, size_t cnt=1);
         
         // ObjectType only, may throw TypeError or MemberError exception
         iterator<Object> obegin();
         const_iterator<Object> obegin() const;
         iterator<Object> oend();
         const_iterator<Object> oend() const;
         iterator<Object> find(const gcore::String &name);
         const_iterator<Object> find(const gcore::String &name) const;
         iterator<Object> find(const char *name);
         const_iterator<Object> find(const char *name) const;
         const Value& operator[](const gcore::String &name) const;
         Value& operator[](const gcore::String &name);
         const Value& operator[](const char *name) const;
         Value& operator[](const char *name);
      
      public:
         
         enum ParserState
         {
            Begin = 0,
            ReadObject,
            ReadObjectKey,
            ReadArray,
            ReadString,
            ReadValue,
            End
         };
         
         struct ParserCallbacks
         {
            gcore::Functor0 objectBegin;
            gcore::Functor1<const char*> objectKey;
            gcore::Functor0 objectEnd;
            gcore::Functor0 arrayBegin;
            gcore::Functor0 arrayEnd;
            gcore::Functor1<bool> booleanScalar;
            gcore::Functor1<double> numberScalar;
            gcore::Functor1<const char*> stringScalar;
            gcore::Functor0 nullScalar;
         };
         
         static void Parse(const char *path, ParserCallbacks *callbacks);
      
      private:
         
         void read(std::istream &is, bool consumeAll, ParserCallbacks *cb);
         
         bool toPropertyList(gcore::PropertyList &pl, const gcore::String &cprop) const;
         
      private:
         
         Type mType;
         union
         {
            bool boo;
            double num;
            gcore::String *str;
            Object *obj;
            Array *arr;
         } mValue;
      };
      
      typedef Value::Object Object;
      typedef Value::iterator<Value::Object> ObjectIterator;
      typedef Value::const_iterator<Value::Object> ObjectConstIterator;
      
      typedef Value::Array Array;
      typedef Value::iterator<Value::Array> ArrayIterator;
      typedef Value::const_iterator<Value::Array> ArrayConstIterator;
      
      class GCORE_API Parser
      {
      public:
         Parser();
         Parser(const char *path);
         ~Parser();
         
         bool read(const char *path);
         
         Value& top();
         const Value& top() const;
         
      private:
         
      
      private:
         Value mTop;
      };
      
      // Schema and Validator
   }
}

GCORE_API std::ostream& operator<<(std::ostream &os, const gcore::json::Value &value);
GCORE_API std::ostream& operator<<(std::ostream &os, const gcore::json::Object &object);
GCORE_API std::ostream& operator<<(std::ostream &os, const gcore::json::Array &array);

#endif
