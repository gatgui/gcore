#include "yaml.h"

namespace gcore
{
namespace yaml
{

Error::Error(const gcore::String &msg)
   : std::exception()
   , mMsg(msg)
{
}

Error::Error(const char *fmt, ...)
   : std::exception()
{
   char msg[1024];
   va_list lst;
   
   va_start(lst, fmt);
   vsnprintf(msg, 1024, fmt, lst);
   va_end(lst);
   
   mMsg = msg;
}

Error::~Error() throw()
{
}

const char* Error::what() const throw()
{
   return mMsg.c_str();
}

// ---

ParserError::ParserError(size_t l, size_t c, const gcore::String &msg)
   : Error(msg)
   , mLine(l)
   , mCol(c)
{
   mMsg += " (line " + gcore::String(l) + ", column: " + gcore::String(c) + ")";
}

ParserError::ParserError(size_t l, size_t c, const char *fmt, ...)
   : Error(" (line " + gcore::String(l) + ", column: " + gcore::String(c) + ")")
   , mLine(l)
   , mCol(c)
{
   char msg[1024];
   va_list lst;
   va_start(lst, fmt);
   vsnprintf(msg, 1024, fmt, lst);
   va_end(lst);
   
   mMsg = msg + mMsg;
}

ParserError::~ParserError() throw()
{
}

size_t ParserError::line() const
{
   return mLine;
}

size_t ParserError::column() const
{
   return mCol;
}

// ---

// YAML Schema
// null | Null | NULL | ~	 tag:yaml.org,2002:null
// /* Empty */	 tag:yaml.org,2002:null
// true | True | TRUE | false | False | FALSE	 tag:yaml.org,2002:bool
// [-+]? [0-9]+	 tag:yaml.org,2002:int (Base 10)
// 0o [0-7]+	 tag:yaml.org,2002:int (Base 8)
// 0x [0-9a-fA-F]+	 tag:yaml.org,2002:int (Base 16)
// [-+]? ( \. [0-9]+ | [0-9]+ ( \. [0-9]* )? ) ( [eE] [-+]? [0-9]+ )?	 tag:yaml.org,2002:float (Number)
// [-+]? ( \.inf | \.Inf | \.INF )	 tag:yaml.org,2002:float (Infinity)
// \.nan | \.NaN | \.NAN	 tag:yaml.org,2002:float (Not a number)
// *	 tag:yaml.org,2002:str (Default)
// anything else -> tag:yaml.org,2002:str

// JSON Schema
// null	 tag:yaml.org,2002:null
// true | false	 tag:yaml.org,2002:bool
// -? ( 0 | [1-9] [0-9]* )	 tag:yaml.org,2002:int
// -? ( 0 | [1-9] [0-9]* ) ( \. [0-9]* )? ( [eE] [-+]? [0-9]+ )?	 tag:yaml.org,2002:float
// *	 Error

// Canonical form:
// tag:yaml.org,2002:null  -> "null"
// tag:yaml.org,2002:bool  -> "true" or "false"
// tag:yaml.org,2002:int   -> decimal value with possibliy leading '-'
// tag:yaml.org,2002:float -> ".inf", "-.inf", ".nan", floating point notation  "-? [1-9] ( \. [0-9]* [1-9] )? ( e [-+] [1-9] [0-9]* )?"
// tag:yaml.org,2002:float

// Other tags
// tag:yaml.org,2002:seq (kind Collection)
// tag:yaml.org,2002:map (kind Collection)
// tag:yaml.org,2002:str (kind Scalar)

Document::Document()
{
}

Document::~Document()
{
   clear();
}

Node Document::addAlias(const gcore::String &name, const Node &node)
{
   if (mAliases.find(name) != mAliases.end())
   {
      return Node();
   }
   
   if (!node.mOwns)
   {
      return Node();
   }
   
   if (node.mIsAlias)
   {
      return Node();
   }
   
   Node* &alias = mAliases[name];
   
   // steal ownership
   alias = new Node(node);
   alias->mIsAlias = true;
   alias->mRefCount = 0;
   alias->mDoc = this;
   
   return Node(alias);
}

void Document::removeAlias(Node *n)
{
   // remove from alias list
   for (AliasMap::iterator it = mAliases.begin(); it != mAliases.end(); ++it)
   {
      if (n == it->second)
      {
         mAliases.erase(it);
         break;
      }
   }
}

Node* Document::getReference(const gcore::String &name) const
{
   AliasMap::const_iterator it = mAliases.find(name);
   return (it != mAliases.end() ? it->second : 0);
}

const char* Document::getAliasName(const Node *n) const
{
   for (AliasMap::const_iterator it = mAliases.begin(); it != mAliases.end(); ++it)
   {
      if (it->second == n)
      {
         return it->first.c_str();
      }
   }
   return 0;
}

size_t Document::getAliasNames(gcore::StringList &names) const
{
   names.clear();
   for (AliasMap::const_iterator it = mAliases.begin(); it != mAliases.end(); ++it)
   {
      names.push(it->first);
   }
   return names.size();
}

void Document::clear()
{
   mTop.clear();
   
   // this should have removed all references to any document alias (ref-counting)
   // at this point, mAliases should be empty
   
   if (mAliases.size() > 0)
   {
      std::cerr << "Alias(es) of the document being destroyed are referenced outside of its own scope." << std::endl;
      std::cerr << "Any such reference is invalid and may result in un-expected program termination." << std::endl;
      for (AliasMap::iterator it = mAliases.begin(); it != mAliases.end(); ++it)
      {
         delete it->second;
      }
      mAliases.clear();
   }
}

Node& Document::top()
{
   return mTop;
}

const Node& Document::top() const
{
   return mTop;
}

bool Document::read(const char *path)
{
   std::ifstream in(path);
   
   if (in.is_open())
   {
      return read(in);
   }
   else
   {
      clear();
      return false;
   }
}

/*
struct Parser
{
   enum State
   {
      ReadBegin = 0
      ReadMappingKey,
      ReadMappingValue,
      ReadSequenceItem,
      ReadLiteral
   };
   
   enum Style
   {
      Block = 0,
      Flow,
      DoubleQuoted, // flow string
      SingleQuoted  // flow string
   };
   
   State state;
   size_t indentWidth;
   gcore::String indent;
   Style style;
   Node *node;
};
*/

// returns true if document read, false otherwise (end of stream)
// incomplete/invalid stream raise exceptions for errors
bool Document::read(std::istream &in)
{
   clear();
   
   /*
   static const char* sSpaces = " \t\v\f\n\r";
   Parser parser;
   // need a stack?
   
   std::string line, str;
   size_t p0, p1;
   size_t len, indent, lineno = 0;
   
   parser.state = ReadBegin;
   //parser.node = &mTop;
   
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
   */
   
   return true;
}

// ---

Node::Node()
   : mType(Node::Null)
   , mOwns(true)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   memset(&mData, 0, sizeof(Data));
}

Node::Node(Node::Type t)
   : mType(Node::Null)
   , mOwns(true)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   memset(&mData, 0, sizeof(Data));
   operator=(t);
}

Node::Node(bool b)
   : mType(Node::Bool)
   , mOwns(true)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   mData.BOOL = b;
}

Node::Node(short s)
   : mType(Node::Integer)
   , mOwns(true)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   mData.INT = (long)s;
}

Node::Node(unsigned short s)
   : mType(Node::Integer)
   , mOwns(true)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   mData.INT = (long)s;
}

Node::Node(int i)
   : mType(Node::Integer)
   , mOwns(true)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   mData.INT = (long)i;
}

Node::Node(unsigned int i)
   : mType(Node::Integer)
   , mOwns(true)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   mData.INT = (long)i;
}

Node::Node(long l)
   : mType(Node::Integer)
   , mOwns(true)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   mData.INT = l;
}

Node::Node(unsigned long l)
   : mType(Node::Integer)
   , mOwns(true)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   mData.INT = (long)l;
}

Node::Node(float f)
   : mType(Node::Float)
   , mOwns(true)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   mData.FLT = (double)f;
}

Node::Node(double d)
   : mType(Node::Float)
   , mOwns(true)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   mData.FLT = d;
}

Node::Node(const char *s)
   : mType(s ? Node::String : Node::Null)
   , mOwns(true)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   mData.STR = (s ? new gcore::String(s) : 0);
}

Node::Node(const std::string &s)
   : mType(Node::String)
   , mOwns(true)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   mData.STR = new gcore::String(s);
}

Node::Node(gcore::String *s)
   : mType(s ? Node::String : Node::Null)
   , mOwns(true)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   mData.STR = s;
}

Node::Node(const Node::Map &m)
   : mType(Node::Mapping)
   , mOwns(true)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   mData.MAP = new Map(m);
}

Node::Node(Node::Map *m)
   : mType(m ? Node::Mapping : Node::Null)
   , mOwns(true)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   mData.MAP = m;
}

Node::Node(const Node::Seq &s)
   : mType(Node::Sequence)
   , mOwns(true)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   mData.SEQ = new Seq(s);
}

Node::Node(Node::Seq *s)
   : mType(s ? Node::Sequence : Node::Null)
   , mOwns(true)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   mData.SEQ = s;
}

Node::Node(Node *r)
   : mType((r && r->isAlias()) ? Node::Reference : Node::Null)
   , mOwns(false)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   mData.REF = r;
   r->ref();
}

Node::Node(const Node &rhs)
   : mType(Node::Null)
   , mOwns(true)
   , mIsAlias(false)
   , mRefCount(0)
   , mDoc(0)
{
   operator=(rhs);
}

Node::~Node()
{
   clear();
}

Node& Node::operator=(Node::Type type)
{
   if (mType == Reference)
   {
      *(mData.REF) = type;
   }
   else
   {
      if (mType != type)
      {
         clear();
         mType = type;
      }
      
      switch (mType)
      {
      case Bool:
         mData.BOOL = false;
         break;
      case Integer:
         mData.INT = 0;
         break;
      case Float:
         mData.FLT = 0.0f;
         break;
      case String:
         if (!mData.STR) mData.STR = new gcore::String("");
         break;
      case Mapping:
         if (!mData.MAP) mData.MAP = new Map();
         break;
      case Sequence:
         if (!mData.SEQ) mData.SEQ = new Seq();
         break;
      case Reference:
         throw Error("Cannot create empty reference");
      default:
         // Null
         break;
      }
   }
   
   return *this;
}

Node& Node::operator=(bool v)
{
   if (mType == Reference)
   {
      *(mData.REF) = v;
   }
   else
   {
      if (mType != Bool)
      {
         clear();
         mType = Bool;
      }
      mData.BOOL = v;
   }
   return *this;
}

Node& Node::operator=(short v)
{
   if (mType == Reference)
   {
      *(mData.REF) = v;
   }
   else
   {
      if (mType != Integer)
      {
         clear();
         mType = Integer;
      }
      mData.INT = (long)v;
   }
   return *this;
}

Node& Node::operator=(unsigned short v)
{
   if (mType == Reference)
   {
      *(mData.REF) = v;
   }
   else
   {
      if (mType != Integer)
      {
         clear();
         mType = Integer;
      }
      mData.INT = (long)v;
   }
   return *this;
}

Node& Node::operator=(int v)
{
   if (mType == Reference)
   {
      *(mData.REF) = v;
   }
   else
   {
      if (mType != Integer)
      {
         clear();
         mType = Integer;
      }
      mData.INT = (long)v;
   }
   return *this;
}

Node& Node::operator=(unsigned int v)
{
   if (mType == Reference)
   {
      *(mData.REF) = v;
   }
   else
   {
      if (mType != Integer)
      {
         clear();
         mType = Integer;
      }
      mData.INT = (long)v;
   }
   return *this;
}

Node& Node::operator=(long v)
{
   if (mType == Reference)
   {
      *(mData.REF) = v;
   }
   else
   {
      if (mType != Integer)
      {
         clear();
         mType = Integer;
      }
      mData.INT = v;
   }
   return *this;
}

Node& Node::operator=(unsigned long v)
{
   if (mType == Reference)
   {
      *(mData.REF) = v;
   }
   else
   {
      if (mType != Integer)
      {
         clear();
         mType = Integer;

      }
      mData.INT = (long)v;
   }
   return *this;
}

Node& Node::operator=(float v)
{
   if (mType == Reference)
   {
      *(mData.REF) = v;
   }
   else
   {
      if (mType != Float)
      {
         clear();
         mType = Float;
      }
      mData.FLT = (double)v;
   }
   return *this;
}

Node& Node::operator=(double v)
{
   if (mType == Reference)
   {
      *(mData.REF) = v;
   }
   else
   {
      if (mType != Float)
      {
         clear();
         mType = Float;
      }
      mData.FLT = v;
   }
   return *this;
}

Node& Node::operator=(const char *v)
{
   if (mType == Reference)
   {
      *(mData.REF) = v;
   }
   else
   {
      if (mType != String)
      {
         clear();
         mType = String;
      }
      
      if (!v)
      {
         clear();
      }
      else if (!mData.STR)
      {
         mData.STR = new gcore::String(v);
      }
      else
      {
         *(mData.STR) = v;
      }
   }
   
   return *this;
}

Node& Node::operator=(const std::string &v)
{
   if (mType == Reference)
   {
      *(mData.REF) = v;
   }
   else
   {
      if (mType != String)
      {
         clear();
         mType = String;
      }
      
      if (!mData.STR)
      {
         mData.STR = new gcore::String(v);
      }
      else
      {
         *(mData.STR) = v;
      }
   }
   
   return *this;
}

Node& Node::operator=(gcore::String *v)
{
   if (mType == Reference)
   {
      *(mData.REF) = v;
   }
   else
   {
      if (mType != String)
      {
         clear();
         mType = String;
      }
      
      if (!v)
      {
         clear();
      }
      else if (!mData.STR)
      {
         mData.STR = v;
      }
      else
      {
         if (v != mData.STR)
         {
            delete mData.STR;
         }
         mData.STR = v;
      }
   }
   
   return *this;
}

Node& Node::operator=(const Node::Map &v)
{
   if (mType == Reference)
   {
      *(mData.REF) = v;
   }
   else
   {
      if (mType != Mapping)
      {
         clear();
         mType = Mapping;
      }
      
      if (!mData.MAP)
      {
         mData.MAP = new Map(v);
      }
      else
      {
         *(mData.MAP) = v;
      }
   }
   
   return *this;
}

Node& Node::operator=(Node::Map *v)
{
   if (mType == Reference)
   {
      *(mData.REF) = v;
   }
   else
   {
      if (mType != Mapping)
      {
         clear();
         mType = Mapping;
      }
      
      if (!v)
      {
         clear();
      }
      else if (!mData.MAP)
      {
         mData.MAP = v;
      }
      else
      {
         if (v != mData.MAP)
         {
            delete mData.MAP;
         }
         mData.MAP = v;
      }
   }
   
   return *this;
}

Node& Node::operator=(const Node::Seq &v)
{
   if (mType == Reference)
   {
      *(mData.REF) = v;
   }
   else
   {
      if (mType != Sequence)
      {
         clear();
         mType = Sequence;
      }
      
      if (!mData.SEQ)
      {
         mData.SEQ = new Seq(v);
      }
      else
      {
         *(mData.SEQ) = v;
      }
   }
   
   return *this;
}

Node& Node::operator=(Node::Seq *v)
{
   if (mType == Reference)
   {
      *(mData.REF) = v;
   }
   else
   {
      if (mType != Sequence)
      {
         clear();
         mType = Sequence;
      }
      
      if (!v)
      {
         clear();
      }
      else if (!mData.SEQ)
      {
         mData.SEQ = v;
      }
      else
      {
         if (v != mData.SEQ)
         {
            delete mData.SEQ;
         }
         mData.SEQ = v;
      }
   }
   
   return *this;
}

Node& Node::operator=(Node *node)
{
   if (!node || !node->isAlias())
   {
      clear();
   }
   else
   {
      if (mType != Reference)
      {
         clear();
         
         mType = Reference;
         mOwns = false;
      }
      else if (mData.REF != node)
      {
         // mType is a reference already, check node
         mData.REF->unref();
      }
      
      mData.REF = node;
      mData.REF->ref();
   }
   return *this;  
}

Node& Node::operator=(const Node &rhs)
{
   if (this != &rhs)
   {
      if (mType != rhs.mType)
      {
         if (mType == Reference && mData.REF == (yaml::Node*) &rhs)
         {
            return *this;
         }
         else
         {
            clear();
         }
      }
      else if (mType == Reference)
      {
         // 'this' and 'rhs' are both references
         if (mData.REF == rhs.mData.REF)
         {
            return *this;
         }
         else
         {
            mData.REF->unref();
         }
      }
      else if (mIsAlias)
      {
         switch (rhs.mType)
         {
         case Null:
            break;
         case Bool:
            mData.BOOL = rhs.mData.BOOL;
            break;
         case Integer:
            mData.INT = rhs.mData.INT;
            break;
         case Float:
            mData.FLT = rhs.mData.FLT;
            break;
         case String:
            *(mData.STR) = *(rhs.mData.STR);
            break;
         case Mapping:
            *(mData.MAP) = *(rhs.mData.MAP);
            break;
         case Sequence:
            *(mData.SEQ) = *(rhs.mData.SEQ);
            break;
         case Reference:
            throw Error("Alias cannot be reference");
         }
         
         // 'this' and 'rhs' ownership don't change
         
         return *this;
      }
      
      // we know for sure 'this' is not an alias
      
      if (rhs.mIsAlias)
      {
         // if 'rhs' is an alias, create a reference
         mType = Reference;
         mData.REF = (yaml::Node*) &rhs;
         mData.REF->ref();
         mOwns = false;
         //mIsAlias = false;
         //mRefCount = 0;
         //mDoc = 0;
         mTag = "";
      }
      else
      {
         mType = rhs.mType;
         mData = rhs.mData;
         mOwns = rhs.mOwns;
         //mIsAlias = false;
         //mRefCount = rhs.mRefCount;
         //mDoc = rhs.mDoc;
         mTag = rhs.mTag;
         
         rhs.mOwns = false;
      }
   }
   
   return *this;
}

Node::operator bool () const
{
   if (mType == Reference)
   {
      return *(mData.REF);
   }
   else if (mType != Bool)
   {
      throw Error("Node is not a boolean");
   }
   return mData.BOOL;
}

Node::operator short () const
{
   if (mType == Reference)
   {
      return *(mData.REF);
   }
   else if (mType != Integer)
   {
      throw Error("Node is not an integer");
   }
   return (short) mData.INT;
}

Node::operator unsigned short () const
{
   if (mType == Reference)
   {
      return *(mData.REF);
   }
   else if (mType != Integer)
   {
      throw Error("Node is not an integer");
   }
   return (unsigned short) mData.INT;
}

Node::operator int () const
{
   if (mType == Reference)
   {
      return *(mData.REF);
   }
   else if (mType != Integer)
   {
      throw Error("Node is not an integer");
   }
   return (int) mData.INT;
}

Node::operator unsigned int () const
{
   if (mType == Reference)
   {
      return *(mData.REF);
   }
   else if (mType != Integer)
   {
      throw Error("Node is not an integer");
   }
   return (unsigned int) mData.INT;
}

Node::operator long () const
{
   if (mType == Reference)
   {
      return *(mData.REF);
   }
   else if (mType != Integer)
   {
      throw Error("Node is not an integer");
   }
   return mData.INT;
}

Node::operator unsigned long () const
{
   if (mType == Reference)
   {
      return *(mData.REF);
   }
   else if (mType != Integer)
   {
      throw Error("Node is not an integer");
   }
   return (unsigned long) mData.INT;
}

Node::operator float () const
{
   if (mType == Reference)
   {
      return *(mData.REF);
   }
   else if (mType != Float)
   {
      throw Error("Node is not a float");
   }
   return (float) mData.FLT;
}

Node::operator double () const
{
   if (mType == Reference)
   {
      return *(mData.REF);
   }
   else if (mType != Float)
   {
      throw Error("Node is not a float");
   }
   return mData.FLT;
}

Node::operator const char* () const
{
   if (mType == Reference)
   {
      return *(mData.REF);
   }
   else if (mType != String)
   {
      throw Error("Node is not a string");
   }
   return mData.STR->c_str();
}

Node::operator const gcore::String& () const
{
   if (mType == Reference)
   {
      return *(mData.REF);
   }
   else if (mType != String)
   {
      throw Error("Node is not a string");
   }
   return *(mData.STR);
}

Node::operator gcore::String& ()
{
   if (mType == Reference)
   {
      return *(mData.REF);
   }
   else if (mType != String)
   {
      throw Error("Node is not a string");
   }
   return *(mData.STR);
}

Node::operator const Node::Map& () const
{
   if (mType == Reference)
   {
      return *(mData.REF);
   }
   else if (mType != Mapping)
   {
      throw Error("Node is not a mapping");
   }
   return *(mData.MAP);
}

Node::operator Node::Map& ()
{
   if (mType == Reference)
   {
      return *(mData.REF);
   }
   else if (mType != Mapping)
   {
      throw Error("Node is not a mapping");
   }
   return *(mData.MAP);
}

Node::operator const Node::Seq& () const
{
   if (mType == Reference)
   {
      return *(mData.REF);
   }
   else if (mType != Sequence)
   {
      throw Error("Node is not a sequence");
   }
   return *(mData.SEQ);
}

Node::operator Node::Seq& ()
{
   if (mType == Reference)
   {
      return *(mData.REF);
   }
   else if (mType != Sequence)
   {
      throw Error("Node is not a sequence");
   }
   return *(mData.SEQ);
}

void Node::clear()
{
   // what if I'm an alias with more reference count!
   if (mIsAlias && mRefCount > 0)
   {
      throw Error("Cannot clear referenced alias");
   }
   
   if (mOwns)
   {
      switch (mType)
      {
      case String:
         if (mData.STR) delete mData.STR;
         break;
      case Mapping:
         if (mData.MAP) delete mData.MAP;
         break;
      case Sequence:
         if (mData.SEQ) delete mData.SEQ;
         break;
      default:
         break;
      }
   }
   
   if (mType == Reference && mData.REF)
   {
      // unref alias
      mData.REF->unref();
   }
   
   mType = Null;
   mRefCount = 0;
   mIsAlias = false;
   mOwns = true;
   mTag = "";
   mDoc = 0;
   
   memset(&mData, 0, sizeof(Data));
}

bool Node::isValid() const
{
   switch (mType)
   {
   case String:
      return (mData.STR != 0);
   case Mapping:
      return (mData.MAP != 0);
   case Sequence:
      return (mData.SEQ != 0);
   case Reference:
      return (mData.REF != 0 && mData.REF->isValid());
   default:
      break;
   }
   
   return true;
}

const char* Node::id() const
{
   if (!isValid())
   {
      return "";
   }
   else if (mType == Reference)
   {
      return mData.REF->id();
   }
   else
   {
      switch (mType)
      {
      case String:
         return mData.STR->c_str();
      
      case Null:
         return "null";
      
      case Bool:
         return (mData.BOOL ? "true" : "false");
      
      case Integer:
         mId = mData.INT;
         break;
      
      case Float:
         mId = mData.FLT;
         break;
      
      case Mapping:
         mId = "{";
         if (mData.MAP->size() > 0)
         {
            Map::const_iterator it = mData.MAP->begin();
            
            mId += it->first.id();
            mId += ":";
            mId += it->second.id();
            
            ++it;
            
            for (; it != mData.MAP->end(); ++it)
            {
               mId += ",";
               mId += it->first.id();
               mId += ":";
               mId += it->second.id();
            }
         }
         mId += "}";
         break;
      
      case Sequence:
         mId = "[";
         if (mData.SEQ->size() > 0)
         {
            Seq::const_iterator it = mData.SEQ->begin();
            
            mId += it->id();
            
            ++it;
            
            for (; it != mData.SEQ->end(); ++it)
            {
               mId += ",";
               mId += it->id();
            }
         }
         mId += "]";
         break;
      
      default:
         break;
      }
      
      return mId.c_str();
   }
}

bool Node::isAlias() const
{
   return mIsAlias;
}

bool Node::isNull() const
{
   return (mType == Null);
}

bool Node::isCollection() const
{
   return (mType == Mapping || mType == Sequence);
}

bool Node::isScalar() const
{
   return (mType != Mapping && mType != Sequence && mType != Reference);
}

void Node::setTag(const char *tag)
{
   if (!tag)
   {
      return;
   }
   
   if (mType == Reference)
   {
      mData.REF->setTag(tag);
   }
   else
   {
      mTag = tag;
   }
}

const char* Node::tag() const
{
   if (mType == Reference)
   {
      return mData.REF->tag();
   }
   else
   {
      if (mTag.length() > 0)
      {
         return mTag.c_str();
      }
      else
      {
         switch (mType)
         {
         case Bool:
            return "tag:yaml.org,2002:bool";
         case Integer:
            return "tag:yaml.org,2002:int";
         case Float:
            return "tag:yaml.org,2002:float";
         case String:
            return "tag:yaml.org,2002:str";
         case Mapping:
            return "tag:yaml.org,2002:map";
         case Sequence:
            return "tag:yaml.org,2002:seq";
         default:
            return "tag:yaml.org,2002:null";
         }
      }
   }
}

void Node::ref()
{
   if (mIsAlias)
   {
      ++mRefCount;
   }
}

void Node::unref()
{
   if (mIsAlias && mRefCount > 0)
   {
      --mRefCount;
      if (mRefCount == 0)
      {
         mDoc->removeAlias(this);
         delete this;
      }
   }
}

const char* Node::getAliasName() const
{
   if (mIsAlias)
   {
      return mDoc->getAliasName(this);
   }
   else if (mType == Reference)
   {
      return mData.REF->getAliasName();
   }
   else
   {
      return 0;
   }
}

void Node::toStream(std::ostream &os, const std::string &indent, bool ignoreFirstIndent) const
{
   // if reference -> first time should output everything with a preceding &
   switch (mType)
   {
   case Null:
      os << (ignoreFirstIndent ? "" : indent) << "null";
      break;
   case Bool:
      os << (ignoreFirstIndent ? "" : indent) << (mData.BOOL ? "true" : "false");
      break;
   case Integer:
      os << (ignoreFirstIndent ? "" : indent) << mData.INT;
      break;
   case Float:
      os << (ignoreFirstIndent ? "" : indent) << mData.FLT;
      break;
   case String:
      os << (ignoreFirstIndent ? "" : indent) << *(mData.STR);
      break;
   case Mapping:
      for (Map::const_iterator it = mData.MAP->begin(); it != mData.MAP->end(); ++it)
      {
         it->first.toStream(os, indent);
         os << ": ";
         if (it->second.isCollection())
         {
            os << std::endl;
            it->second.toStream(os, indent+"  ");
         }
         else
         {
            it->second.toStream(os, indent+"  ", true);
         }
         os << std::endl;
      }
      break;
   case Sequence:
      for (Seq::const_iterator it = mData.SEQ->begin(); it != mData.SEQ->end(); ++it)
      {
         os << indent << "- ";
         it->toStream(os, indent+"  ", true);
         os << std::endl;
      }
      break;
   case Reference:
      os << (ignoreFirstIndent ? "" : indent) << "&" << getAliasName() << " ";
      mData.REF->toStream(os, indent, true);
      //os << (ignoreFirstIndent ? "" : indent) << "*" << getAliasName();
      break;
   }
}

} // namespace yaml

} // namespace gcore

// ---

using namespace gcore;

int main(int argc, char **argv)
{
   std::cout << "YAML test" << std::endl;
   
   /*
   yaml::Node nTop(yaml::Node::Mapping);
   yaml::Node nMaya(yaml::Node::Mapping);
   yaml::Node nRend(yaml::Node::Sequence);
   
   yaml::Node::Map &opts = nTop;
   yaml::Node::Map &maya = nMaya;
   yaml::Node::Seq &rend = nRend;
   */
   yaml::Node::Map opts;
   yaml::Node::Map maya;
   yaml::Node::Seq rend;
   
   rend.push_back("arnold");
   rend.push_back("vray");
   
   maya["exportAll"] = true;
   maya["supportedRenderers"] = rend;
   maya["scale"] = 0.1;
   
   opts["maya"] = maya;
   
   yaml::Node(opts).toStream(std::cout);
   std::cout << std::endl;
   
   return 0;
}
