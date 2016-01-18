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

#ifndef __gcore_yaml_h_
#define __gcore_yaml_h_

#include <gcore/string.h>

namespace gcore
{
   namespace yaml
   {
      class GCORE_API Error : public std::exception
      {
      public:
         explicit Error(const gcore::String &msg);
         explicit Error(const char *fmt, ...);
         virtual ~Error() throw();
         
         const char* what() const throw();
         
      protected:
         gcore::String mMsg;
      };
      
      class GCORE_API ParserError : public Error
      {
      public:
         explicit ParserError(size_t l, size_t c, const gcore::String &msg);
         explicit ParserError(size_t l, size_t c, const char *fmt, ...);
         virtual ~ParserError() throw();
         
         size_t line() const;
         size_t column() const;
      
      protected:
         size_t mLine;
         size_t mCol;
      };
      
      // ---
      
      class Document;
      
      class GCORE_API Node
      {
      public:
         
         friend class Document;
         
         struct NodeCompare
         {
            inline bool operator()(const Node &n0, const Node &n1) const
            {
               return (strcmp(n0.id(), n1.id()) < 0);
            }
         };
         
         typedef std::map<Node, Node, NodeCompare> Map;
         typedef gcore::List<Node> Seq;
         
         enum Type
         {
            Null = 0,
            Bool,
            Integer,
            Float,
            String,
            Mapping,
            Sequence,
            Reference
         };
         
      public:
         
         Node();
         Node(Type t);
         Node(bool b);
         Node(short s);
         Node(unsigned short s);
         Node(int i);
         Node(unsigned int i);
         Node(long l);
         Node(unsigned long l);
         Node(float f);
         Node(double d);
         Node(const char *s);
         Node(const std::string &s);
         Node(gcore::String *s);
         Node(const Map &m);
         Node(Map *m);
         Node(const Seq &s);
         Node(Seq *s);
         Node(Node *r);
         Node(const Node &rhs);
         ~Node();
         
         Node& operator=(Type type);
         Node& operator=(bool v);
         Node& operator=(short v);
         Node& operator=(unsigned short v);
         Node& operator=(int v);
         Node& operator=(unsigned int v);
         Node& operator=(long v);
         Node& operator=(unsigned long v);
         Node& operator=(float v);
         Node& operator=(double v);
         Node& operator=(const char *v);
         Node& operator=(const std::string &v);
         Node& operator=(gcore::String *v);
         Node& operator=(const Map &v);
         Node& operator=(Map *v);
         Node& operator=(const Seq &v);
         Node& operator=(Seq *v);
         Node& operator=(Node *node);
         Node& operator=(const Node &rhs);
         
         operator bool () const;
         operator short () const;
         operator unsigned short () const;
         operator int () const;
         operator unsigned int () const;
         operator long () const;
         operator unsigned long () const;
         operator float () const;
         operator double () const;
         operator const char* () const;
         operator const gcore::String& () const;
         operator gcore::String& ();
         operator const Map& () const;
         operator Map& ();
         operator const Seq& () const;
         operator Seq& ();
         
         bool isValid() const;
         bool isAlias() const;
         bool isNull() const;
         bool isCollection() const;
         bool isScalar() const;
         
         const char* getAliasName() const;
         
         const char* tag() const;
         void setTag(const char *tag);
         
         void clear();
         const char* id() const;
         
         void toStream(std::ostream &os, const std::string &indent="", bool ignoreFirstIndent=false) const;
         
      private:
         
         union Data
         {
            bool BOOL;
            long INT;
            double FLT;
            gcore::String *STR;
            Map *MAP;
            Seq *SEQ;
            Node *REF;
         };
         
         void ref();
         void unref();
         
      private:
         
         Type mType;
         Data mData;
         mutable gcore::String mId;
         mutable bool mOwns;
         
         bool mIsAlias;
         size_t mRefCount;
         
         Document *mDoc;
         gcore::String mTag;
      };
      
      class GCORE_API Document
      {
      public:
         
         typedef std::map<gcore::String, Node*> AliasMap;
      
      public:
         
         Document();
         ~Document();
         
         Node addAlias(const gcore::String &name, const Node &node);
         void removeAlias(Node *n);
         
         Node* getReference(const gcore::String &name) const;
         const char* getAliasName(const Node *n) const;
         size_t getAliasNames(gcore::StringList &names) const;
         
         void clear();
         
         // returns true if a document has been read successfully
         // both functions will only read a single document
         bool read(const char *path);
         bool read(std::istream &in);
         
         Node& top();
         const Node& top() const;
         
      private:
         
         Node mTop;
         AliasMap mAliases;
      };
   }

}

#endif
