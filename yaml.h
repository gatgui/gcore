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
      class Error : public std::exception
      {
      public:
         explicit Error(const gcore::String &msg);
         explicit Error(const char *fmt, ...);
         virtual ~Error() throw();
         
         const char* what() const throw();
         
      protected:
         gcore::String mMsg;
      };
      
      class ParserError : public Error
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
      
      class GCORE_API Node
      {
      public:
         
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
         
         typedef std::map<gcore::String, Node*> AliasMap;
      
      public:
         
         // This is not enough, what if we have several documents...
         // Each of them may have aliases with same name but different value
         //   isn't it?
         // => Document class?
         static Node AddAlias(const gcore::String &name, const Node &node);
         static Node* GetReference(const gcore::String &name);
         static const char* GetAliasName(Node *n);
         
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
         
         static AliasMap msAliases;
         
         Type mType;
         Data mData;
         mutable gcore::String mId;
         mutable bool mOwns;
         
         bool mIsAlias;
         size_t mRefCount;
         
         //gcore::String mTag;
         //Style mStyle; -> block, flow (single/double quoted)
         //Document?
      };
      
      /*
      void Parse(const char *path)
      {
         std::ifstream in(path);
         if (path.is_open())
         {
            Parse(in);
         }
      }
      
      struct Parser
      {
         enum State
         {
            DocumentBegin = 0
            ReadMappingKey,
            ReadMappingValue,
            ReadSequenceItem,
            ReadLiteral
         };
         
         enum Style
         {
            Block = 0,
            Flow
         };
         
         State state;
         size_t indentWidth;
         gcore::String indent;
         Style style;
         gcore::String tag;
         Node *node; // has tag and style
      };
      
      // returns true if document read, false otherwise (end of stream)
      // incomplete/invalid stream raise exceptions for errors
      bool Parse(std::ifstream &in, Node &out)
      {
         static const char* sSpaces = " \t\v\f\n\r";
         Parser parser;
         // need a stack?
         
         std::string line, str;
         size_t p0, p1;
         size_t len, indent, lineno = 0;
         
         while (in.good())
         {
            if (line.length() == 0)
            {
               std::getline(line);
               ++lineno;
               
               p0 = 0;
               indent = 0;
               len = line.length();
               
               while (p0 < len && line[p0] == ' ')
               {
                  ++p0;
               }
               
               indent = p0;
            }
            
            if (p0 >= len)
            {
               // empty line
               // if reading a scalar, add to str buffer
               if (parser.State == ReadLiteral)
               {
                  if (indent > parser.indentWidth)
                  {
                     if (parser.style == Parser::Block)
                     {
                        
                     }
                     else
                     {
                        // more indented than previous 
                     }
                  }
               }
            }
            else if (line[p0] == '#')
            {
               // ignore line
               line = "";
            }
            else if (line[p0] == '%')
            {
               // ignore directives
               line = "";
            }
            else if (line[p0] == '-' &&
                     p0+1 < len && line[p0+1] == '-' &&
                     p0+2 < len && line[p0+2] == '-')
            {
               // begin of document
               // what about the remaining of the line?
            }
            else if (line[p0] == '.' &&
                     p0+1 < len && line[p0+1] == '.' &&
                     p0+2 < len && line[p0+2] == '.')
            {
               return true;
            }
            else if (line[p0] == '?')
            {
               // mapping key
            }
            else if (line[p0] == ':')
            {
               // mapping value
            }
            else if (line[p0] == '-')
            {
               // sequence item
            }
            else if (line[p0] == '"' || line[p0] == '\'')
            {
               // single/double quoted string
               // double quotes allow character escaping
               
               // has to be single line when used as a key
            }
            else if (line[p0] == '!')
            {
               // tag
               // support: !!map, !!seq, !!str, !!bool, !!int, !!float, !!binary
               // just keep info
               p1 = line.find_first_of(sSpaces, p0);
               
               if (p1 == std::string::npos)
               {
                  raise ParserError(lineno, p0+1, "Unfinished tag?");
               }
               
               tag = line.substr(p0+1, p1-p0-1);
               
               line = line.substr(p1+1);
               p0 = 0;
            }
            else if (line[p0] == '&')
            {
               // anchor
            }
            else if (line[p0] == '*')
            {
               // reference
            }
            else if (line[p0] == '{')
            {
               
            }
            else if (line[p0] == '}')
            {
               
            }
            else if (line[p0] == '[')
            {
               
            }
            else if (line[p0] == ']')
            {
               
            }
            else if (line[p0] == ',')
            {
               
            }
            else if (line[p0] == '@' || line[p0] == '`')
            {
               // reserved !
               raise ParserError(lineno, p0+1, "Reserved marker");
            }
            else
            {
               // line folding rules
               // -> when to preserve \n
               // -> more indented lines
               // -> stripping of white spaces
               // -> etc..
               // ignore tokens in block/flow literals
            }
         }
         
         // reached end of stream without error
         // check current state?
         if (parser.state != Parse::ReadMappingKey)
         {
            // ?
         }
         
         return true;
      }
      */
   }

}

#endif
