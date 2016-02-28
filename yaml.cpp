#include "yaml.h"
#include <gcore/rex.h>

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

// ---

// yaml schema
static gcore::Rex gsInt10E(RAW("^[-+]?[0-9]+$"));
static gcore::Rex gsInt8E(RAW("^0o[0-7]+$"));
static gcore::Rex gsInt16E(RAW("0x[0-9a-fA-F]+$"));
static gcore::Rex gsFltE(RAW("^[-+]?(\.[0-9]+|[0-9]+(\.[0-9]+)?)([eE][-+]?[0-9]+)?$"));

// json schema
static gcore::Rex gsInt10Eb(RAW("^-?(0|[1-9][0-9]*)$"));
static gcore::Rex gsFltEb(RAW("^-?(0|[1-9][0-9]*)(\.[0-9]*)?([eE][-+]?[0-9]+)?$"));


//static const char* gsAllIndicators = "-?:,[]{}#&*!|>'\"%@`";
//static const char* gsFlowIndicators = ",[]{}";
//static const char* gsReservedIndicators = "@`";
static const char* gsWhiteSpace = " \t";
//static const char* gsUriChars = "#;/?:@&=+$,_.!~*'()[]";


struct ParserState
{
   typedef gcore::List<ParserState, false> Stack;
   
   enum Chomp
   {
      Strip = 0,
      Clip,
      Keep
   };
   
   ParserState()
      : inFlowStr(false)
      , inFlowSeq(false)
      , inFlowMap(false)
      , inBlock(false)
      , indentWidth(0)
      , indent("")
      , chomp(Keep)
      , fold(false)
      , tag("")
      , alias("")
      , node(0)
      , allowEscape(false)
   {
   }
   
   ParserState(const ParserState &rhs)
      : inFlowStr(rhs.inFlowStr)
      , inFlowSeq(rhs.inFlowSeq)
      , inFlowMap(rhs.inFlowMap)
      , inBlock(rhs.inBlock)
      , indentWidth(rhs.indentWidth)
      , indent(rhs.indent)
      , chomp(rhs.chomp)
      , fold(rhs.fold)
      , tag(rhs.tag)
      , alias(rhs.alias)
      , node(rhs.node)
      , allowEscape(rhs.allowEscape)
   {
   }
   
   ParserState& operator=(const ParserState &rhs)
   {
      if (this != &rhs)
      {
         inFlowStr = rhs.inFlowStr;
         inFlowSeq = rhs.inFlowSeq;
         inFlowMap = rhs.inFlowMap;
         inBlock = rhs.inBlock;
         indentWidth = rhs.indentWidth;
         indent = rhs.indent;
         chomp = rhs.chomp;
         fold = rhs.fold; // when inBlock is true
         tag = rhs.tag;
         alias = rhs.alias;
         node = rhs.node;
         allowEscape = rhs.allowEscape;
      }
      return *this;
   }
   
   gcore::String contentString(const gcore::StringList &lst)
   {
      gcore::String rv, line;
      size_t n = lst.size();
      
      for (size_t i=0; i<n; ++i)
      {
         line = lst[i];
         if (inFlowStr)
         {
            // Note: preserves leading and trailing white spaces in flow strings
            //       "  hello "
            //       ' goodbye  '
            // => only strip white spaces if the string spans multiple lines
            if (n > 1)
            {
               if (i == 0) line.rstrip();
               else if (i + 1 == n) line.lstrip();
               else line.strip();
            }
            
            if (i > 0) rv.push_back(' ');
            rv += line;
         }
         else if (inBlock)
         {
            // remove indentation
            // TODO
            
            if (fold)
            {
               line.rstrip();
               if (i > 0) rv.push_back(' ');
               rv += line;
            }
            else
            {
               line.rstrip();
               if (i > 0) rv.push_back('\n');
               rv += line;
            }
         }
         else
         {
            // only use last line
            rv = line;
         }
      }
      
      if (inBlock)
      {
         switch (chomp)
         {
         case Keep:
            break;
         case Strip:
            rv.rstrip();
            break;
         case Clip:
            // only keep one trailing newline (if any)
         default:
            break;
         }
      }
      
      return rv;
   }
   
   bool inFlowStr;
   bool inFlowSeq;
   bool inFlowMap;
   bool inBlock;
   size_t indentWidth;
   gcore::String indent;
   Chomp chomp;
   bool fold;
   gcore::String tag;
   gcore::String alias;
   Node *node;
   bool allowEscape;
};

// returns true if document read, false otherwise (end of stream)
// incomplete/invalid stream throw exceptions for errors
bool Document::read(std::istream &in)
{
   clear();
   
   ParserState::Stack stateStack;
   
   ParserState initialState;
   ParserState *state = &initialState;
   
   gcore::StringList content;
   gcore::String line, str, indent;
   size_t p0, p1;
   size_t len, indentWidth, colno = 1, lineno = 0;
   
   while (in.good())
   {
      if (line.length() == 0)
      {
         std::getline(in, line);
         
         ++lineno;
         colno = 1;
         p0 = 0;
         len = line.length();
         
         // figure out indent width
         indentWidth = 0;
         
         // Note: indent width may be impose in block values
         //       => don't deal with it here
         
         while (p0 < len && line[p0] == ' ')
         {
            ++p0;
         }
         
         indentWidth = p0;
         indent = indentWidth * String(" ");
         
         // consume any additional white spaces (should I? [see below])
         p1 = line.find_first_not_of(gsWhiteSpace, p0);
         p0 = (p1 != std::string::npos ? p1 : len);
         
         #ifdef _DEBUG
         std::cerr << "[Document::read] Read new line '" << line << "'" << std::endl;
         std::cerr << "[Document::read]   Indent = '" << indent << "' [" << indentWidth << "]" << std::endl;
         //std::cerr << "[Document::read]   Skip white spaces to " << p1 << std::endl;
         #endif
      }
      
      if (p0 >= len)
      {
         #ifdef _DEBUG
         std::cerr << "[Document::read] Reached EOL" << std::endl;
         std::cerr << "[Document::read]   str = '" << str << "'" << std::endl;
         #endif
         
         content.push(str);
         str = "";
         
         line = "";
      }
      else if (line[p0] == '#') // ignore inside a flow string
      {
         // ignore comments, unless contained in a single/double quoted string
         #ifdef _DEBUG
         std::cerr << "[Document::read] Skip comment [" << lineno << ", " << (colno + p0) << "]" << std::endl;
         #endif
         
         line = "";
      }
      else if (line[p0] == '%') // only at the start of a line
      {
         // ignore directives (should I validate them just in case)
         #ifdef _DEBUG
         std::cerr << "[Document::read] Skip directive [" << lineno << ", " << (colno + p0) << "]" << std::endl;
         #endif
         
         line = "";
      }
      else if (line[p0] == '-' &&
               p0+1 < len && line[p0+1] == '-' &&
               p0+2 < len && line[p0+2] == '-') // only at the start of a line
      {
         #ifdef _DEBUG
         std::cerr << "[Document::read] Begin of document [" << lineno << ", " << (colno + p0) << "]" << std::endl;
         std::cerr << "[Document::read]   str = '" << str << "'" << std::endl;
         #endif
         
         str = "";
         line = "";
         
         // begin of document
         if (stateStack.size() > 0) // rather check for state->node
         {
            // complete current document
            #ifdef _DEBUG
            std::cerr << "[Document::read]   Begin of next document" << std::endl;
            #endif
            break;
         }
         else
         {
            state->node = &mTop;
         }
      }
      else if (line[p0] == '.' &&
               p0+1 < len && line[p0+1] == '.' &&
               p0+2 < len && line[p0+2] == '.') // only at the start of a line
      {
         #ifdef _DEBUG
         std::cerr << "[Document::read] End of document [" << lineno << ", " << (colno + p0) << "]" << std::endl;
         std::cerr << "[Document::read]   str = '" << str << "'" << std::endl;
         #endif
         
         line = "";
         str = "";
         
         // end of document
         break;
      }
      else if (line[p0] == '!')
      {
         // tag
         p1 = line.find_first_of(gsWhiteSpace, p0);
         
         size_t len = 0;
         
         if (p1 == std::string::npos)
         {
            if (p0 + 1 < line.length())
            {
               len = line.length() - p0 - 1;
            }
         }
         else
         {
            if (p0 + 1 < p1)
            {
               len = p1 - p0 - 1;
            }
         }
         
         if (len == 0)
         {
            throw ParserError(lineno, colno + p0, "Incomplete tag");
         }
         
         state->tag = line.substr(p0+1, len);
         
         #ifdef _DEBUG
         std::cerr << "[Document::read] Read tag '" << state->tag << "' [" << lineno << ", " << (colno + p0) << "]" << std::endl;
         std::cerr << "[Document::read]   str = '" << str << "'" << std::endl;
         #endif
         
         str = "";
         
         if (p1 == std::string::npos)
         {
            line = "";
            colno = 1;
         }
         else
         {
            line = line.substr(p1+1);
            colno += p1 + 1;
         }
         p0 = 0;
      }
      else if (line[p0] == '&')
      {
         // if not reading a scalar?
         // alias anchor
         p1 = line.find_first_of(gsWhiteSpace, p0);
         
         size_t len = 0;
         
         if (p1 == std::string::npos)
         {
            if (p0 + 1 < line.length())
            {
               len = line.length() - p0 - 1;
            }
         }
         else
         {
            if (p0 + 1 < p1)
            {
               len = p1 - p0 - 1;
            }
         }
         
         if (len == 0)
         {
            throw ParserError(lineno, colno + p0, "Incomplete alias");
         }
         
         state->alias = line.substr(p0+1, len);
         
         #ifdef _DEBUG
         std::cerr << "[Document::read] Read alias '" << state->alias << "' [" << lineno << ", " << (colno + p0) << "]" << std::endl;
         std::cerr << "[Document::read]   str = '" << str << "'" << std::endl;
         #endif
         
         str = "";
         
         if (p1 == std::string::npos)
         {
            line = "";
            colno = 1;
         }
         else
         {
            line = line.substr(p1+1);
            colno += p1 + 1;
         }
         p0 = 0;
      }
      else if (line[p0] == '*')
      {
         // reference
         p1 = line.find_first_of(gsWhiteSpace, p0);
         
         if (p1 == std::string::npos)
         {
            throw ParserError(lineno, colno + p0, "Incomplete alias");
         }
         
         std::string name = line.substr(p0+1, p1-p0-1);
         
         #ifdef _DEBUG
         std::cerr << "[Document::read] Read reference '" << name << "' [" << lineno << ", " << (colno + p0) << "]" << std::endl;
         std::cerr << "[Document::read]   str = '" << str << "'" << std::endl;
         #endif
         
         Node *node = getReference(name);
         
         if (!node)
         {
            throw ParserError(lineno, colno + p0, "Undefined reference '" + name + "'");
         }
         
         // if reading a key -> set key and get not
         // if reading a sequence -> add item
         str = "";
         
         line = line.substr(p1+1);
         colno += p1 + 1;
         p0 = 0;
      }
      else if (line[p0] == '?')
      {
         #ifdef _DEBUG
         std::cerr << "[Document::read] Read mapping key indicator [" << lineno << ", " << (colno + p0) << "]" << std::endl;
         std::cerr << "[Document::read]   str = '" << str << "'" << std::endl;
         #endif
         
         // mapping key
         str = "";
         p0++;
      }
      else if (line[p0] == ':')
      {
         #ifdef _DEBUG
         std::cerr << "[Document::read] Read mapping value indicator [" << lineno << ", " << (colno + p0) << "]" << std::endl;
         std::cerr << "[Document::read]   str = '" << str << "'" << std::endl;
         #endif
         // mapping value
         // use both for block and flow styles
         str.strip();
         if (str.length() == 0)
         {
            throw ParserError(lineno, colno + p0, "Empty mapping key");
         }
         
         // infer type unless specially tagged
         if (state->tag.length() == 0)
         {
            // see above
            // match fixed values first
            // true false null .nan .inf -.inf
            // then integer (using expression)
            // then float (using expression)
            // -> str if all fails
         }
         else
         {
            // parse as asked
            //   !str
            //   !bool
            //   !float
            //   !int
            //   !binary (base64 encoded binary data)
            //   !seq
            //   !map
            // => default to !str if unknown? or back to inference
         }
         
         str = "";
         p0++;
      }
      else if (line[p0] == '-')
      {
         #ifdef _DEBUG
         std::cerr << "[Document::read] Read sequence item indicator [" << lineno << ", " << (colno + p0) << "]" << std::endl;
         std::cerr << "[Document::read]   str = '" << str << "'" << std::endl;
         #endif
         // sequence item
         
         str = "";
         p0++;
      }
      else if (line[p0] == '"' || line[p0] == '\'') // only not inside a flow string or a block value
      {
         #ifdef _DEBUG
         std::cerr << "[Document::read] Read string indicator [" << lineno << ", " << (colno + p0) << "]" << std::endl;
         std::cerr << "[Document::read]   str = '" << str << "'" << std::endl;
         #endif
         // single/double quoted string
         // double quotes allow character escaping
         
         str = "";
         p0++;
      }
      else if (line[p0] == '{') // only if not inside a flow string or a block value
      {
         #ifdef _DEBUG
         std::cerr << "[Document::read] Start new mapping [" << lineno << ", " << (colno + p0) << "]" << std::endl;
         std::cerr << "[Document::read]   str = '" << str << "'" << std::endl;
         #endif
         
         // new dictionary
         str = "";
         p0++;
      }
      else if (line[p0] == '}') // only if inside a flow mapping
      {
         #ifdef _DEBUG
         std::cerr << "[Document::read] End mapping [" << lineno << ", " << (colno + p0) << "]" << std::endl;
         std::cerr << "[Document::read]   str = '" << str << "'" << std::endl;
         #endif
         
         str = "";
         p0++;
         
         // close dict
         ParserState newState;
         
         newState.node = new Node(Node::Mapping);
         newState.node->setTag(state->tag);
         // tag?
         // alias?
         // indent?
         // chomp?
         // fold?
         
         stateStack.push(newState);
         
         state = &(stateStack.back());
      }
      else if (line[p0] == '[') // only if not inside a flow string or a block value
      {
         #ifdef _DEBUG
         std::cerr << "[Document::read] Start new sequence [" << lineno << ", " << (colno + p0) << "]" << std::endl;
         std::cerr << "[Document::read]   str = '" << str << "'" << std::endl;
         #endif
         
         str = "";
         p0++;
         
         // new sequence
         ParserState newState;
         
         newState.node = new Node(Node::Sequence);
         
         stateStack.push(newState);
         
         state = &(stateStack.back());
      }
      else if (line[p0] == ']') // only if inside a flow sequence
      {
         #ifdef _DEBUG
         std::cerr << "[Document::read] End sequence [" << lineno << ", " << (colno + p0) << "]" << std::endl;
         std::cerr << "[Document::read]   str = '" << str << "'" << std::endl;
         #endif
         
         str = "";
         p0++;
         
         // end of sequence... set key value?
         // -> should not have to
      }
      else if (line[p0] == ',') // only if inside a flow sequence or mapping
      {
         #ifdef _DEBUG
         std::cerr << "[Document::read] Add new mapping/sequence item [" << lineno << ", " << (colno + p0) << "]" << std::endl;
         std::cerr << "[Document::read]   str = '" << str << "'" << std::endl;
         #endif
         
         
         str = "";
         p0++;
      }
      else if (line[p0] == '>')
      {
         // may have '+', '-', or \d+ behind
         #ifdef _DEBUG
         std::cerr << "[Document::read] Begin block value (>) [" << lineno << ", " << (colno + p0) << "]" << std::endl;
         std::cerr << "[Document::read]   str = '" << str << "'" << std::endl;
         #endif
         
         str = "";
         p0++;
      }
      else if (line[p0] == '|')
      {
         // may have '+', '-', or \d+ behind
         #ifdef _DEBUG
         std::cerr << "[Document::read] Begin block value (|) [" << lineno << ", " << (colno + p0) << "]" << std::endl;
         std::cerr << "[Document::read]   str = '" << str << "'" << std::endl;
         #endif
         
         str = "";
         p0++;
      }
      else if (line[p0] == '@' || line[p0] == '`')
      {
         // reserved !
         #ifdef _DEBUG
         std::cerr << "[Document::read] Reserved indicator [" << lineno << ", " << (colno + p0) << "]" << std::endl;
         #endif
         
         throw ParserError(lineno, colno + p0, "Reserved marker");
      }
      else
      {
         #ifdef _DEBUG
         std::cerr << "[Document::read] Appending character '" << line[p0] << "'" << std::endl;
         #endif
         
         str.push_back(line[p0++]);
      }
   }
   
   // reached end of stream without error
   // check current state?
   return (mTop.isValid() && mTop.type() == Node::Mapping);
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

void Node::clearValue()
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
   
   memset(&mData, 0, sizeof(Data));
}

void Node::clear()
{
   clearValue();
   
   mTag = "";
   mDoc = 0;
}

Node::Type Node::type() const
{
   return mType;
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

void Node::setTag(const std::string &s)
{
   setTag(s.c_str());
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

bool Node::fromString(const gcore::String &stringValue, bool noexc)
{
   clearValue();
   
   // YAML Schema
   //   null | Null | NULL | ~	 tag:yaml.org,2002:null
   //   /* Empty */	 tag:yaml.org,2002:null
   //   true | True | TRUE | false | False | FALSE	 tag:yaml.org,2002:bool
   //   [-+]? [0-9]+	 tag:yaml.org,2002:int (Base 10)
   //   0o [0-7]+	 tag:yaml.org,2002:int (Base 8)
   //   0x [0-9a-fA-F]+	 tag:yaml.org,2002:int (Base 16)
   //   [-+]? ( \. [0-9]+ | [0-9]+ ( \. [0-9]* )? ) ( [eE] [-+]? [0-9]+ )?	 tag:yaml.org,2002:float (Number)
   //   [-+]? ( \.inf | \.Inf | \.INF )	 tag:yaml.org,2002:float (Infinity)
   //   \.nan | \.NaN | \.NAN	 tag:yaml.org,2002:float (Not a number)
   //   *	 tag:yaml.org,2002:str (Default)
   
   // JSON Schema
   //   null	 tag:yaml.org,2002:null
   //   true | false	 tag:yaml.org,2002:bool
   //   -? ( 0 | [1-9] [0-9]* )	 tag:yaml.org,2002:int
   //   -? ( 0 | [1-9] [0-9]* ) ( \. [0-9]* )? ( [eE] [-+]? [0-9]+ )?	 tag:yaml.org,2002:float
   //   *	 Error
   
   //bool boolValue = false;
   long intValue = 0;
   double floatValue = 0.0;
   
   if (mTag.length() == 0)
   {
      size_t len = stringValue.length();
      double sign = 1;
      
      if (len > 0)
      {
         size_t ci = 0;
         char c0 = stringValue[0];
         char c1 = (len > 1 ? stringValue[1] : 0);
         char c2 = (len > 2 ? stringValue[2] : 0);
         char c3 = (len > 3 ? stringValue[3] : 0);
         char c4 = (len > 4 ? stringValue[4] : 0);
         
         switch (c0)
         {
         case 'T':
         case 't':
            if (len == 4)
            {
               if ((c0 == 'T' && c1 == 'R' && c2 == 'U' && c3 == 'E') ||
                                (c1 == 'r' && c2 == 'u' && c3 == 'e'))
               {
                  operator=(true);
                  return true;
               }
            }
            operator=(stringValue);
            return true;
         case 'F':
         case 'f':
            if (len == 5)
            {
               if ((c0 == 'F' && c1 == 'A' && c2 == 'L' && c3 == 'S' && c4 == 'E') ||
                                (c1 == 'a' && c2 == 'l' && c3 == 's' && c4 == 'e'))
               {
                  operator=(false);
                  return true;
               }
            }
            operator=(stringValue);
            return true;
         case 'N':
         case 'n':
            if (len == 4)
            {
               if ((c0 == 'N' && c1 == 'U' && c2 == 'L' && c3 == 'L') ||
                                (c1 == 'u' && c2 == 'l' && c3 == 'l'))
               {
                  operator=(Null);
                  return true;
               }
            }
            operator=(stringValue);
            return true;
         case '-':
            sign = -1;
         case '+':
            if (len == 1 || stringValue[1] != '.')
            {
               // can still be a float/integer number
               break;
            }
            // c0 = '+' / '-'
            // c1 = '.'
            ++ci;
            c1 = c2;
            c2 = c3;
            c3 = c4;
         case '.':
            // check length taking sign into account
            if (len == 4 + ci)
            {
               switch (c1)
               {
               case 'n':
               case 'N':
                  if ((c1 == 'N' && (c2 == 'a' || c2 == 'A') && c3 == 'N') ||
                      (c1 == 'n' &&  c2 == 'a'               && c3 == 'n'))
                  {
                     operator=(std::numeric_limits<double>::signaling_NaN());
                     return true;
                  }
                  break;
               case 'i':
               case 'I':
                  if ((c1 == 'I' && c2 == 'N' && c3 == 'F') ||
                                   (c2 == 'n' && c3 == 'f'))
                  {
                     operator=(sign * std::numeric_limits<double>::infinity());
                     return true;
                  }
               default:
                  break;
               }
            }
         default:
            break;
         }
         
         if (gsInt10E.match(stringValue) || gsInt8E.match(stringValue) ||
             gsInt16E.match(stringValue))
         {
            sscanf(stringValue.c_str(), "%ld", &intValue);
            operator=(intValue);
         }
         else if (gsFltE.match(stringValue))
         {
            sscanf(stringValue.c_str(), "%lf", &floatValue);
            operator=(floatValue);
         }
         else
         {
            operator=(stringValue);
         }
      }
      else
      {
         operator=(Null);
      }
   }
   else
   {
      size_t p = mTag.rfind(':');
      gcore::String tag = (p != std::string::npos ? mTag.substr(p + 1) : mTag);
      
      if (tag == "float")
      {
         if (!stringValue.toDouble(floatValue))
         {
            if (noexc)
            {
               return false;
            }
            else
            {
               throw Error("'%s' cannot be converted to '%s'", stringValue.c_str(), mTag.c_str());
            }
         }
         operator=(floatValue);
      }
      else if (tag == "int")
      {
         if (!stringValue.toLong(intValue))
         {
            if (noexc)
            {
               return false;
            }
            else
            {
               throw Error("'%s' cannot be converted to '%s'", stringValue.c_str(), mTag.c_str());
            }
         }
         operator=(intValue);
      }
      else if (tag == "bool")
      {
         size_t len = stringValue.length();
         if (len == 4 || len == 5)
         {
            char c0 = stringValue[0];
            char c1 = stringValue[1];
            char c2 = stringValue[2];
            char c3 = stringValue[3];
            if (len == 5)
            {
               char c4 = stringValue[4];
               if (c0 == 'F')
               {
                  if ((c1 == 'A' && c2 == 'L' && c3 == 'S' && c4 == 'E') ||
                      (c1 == 'a' && c2 == 'l' && c3 == 's' && c4 == 'e'))
                  {
                     operator=(false);
                  }
               }
               else if (c0 == 'f' && c1 == 'a' && c2 == 'l' && c3 == 's' && c4 == 'e')
               {
                  operator=(false);
               }
            }
            else
            {
               if (c0 == 'T')
               {
                  if ((c1 == 'R' && c2 == 'U' && c3 == 'E') ||
                      (c1 == 'r' && c2 == 'u' && c3 == 'e'))
                  {
                     operator=(true);
                  }
               }
               else if (c0 == 't' && c1 == 'r' && c2 == 'u' && c3 == 'e')
               {
                  operator=(true);
               }
            }
         }
         if (type() != Bool)
         {
            if (noexc)
            {
               return false;
            }
            else
            {
               throw Error("'%s' cannot be converted to '%s'", stringValue.c_str(), mTag.c_str());
            }
         }
      }
      else if (tag == "null")
      {
         operator=(Null);
      }
      else if (tag == "str")
      {
         operator=(stringValue);
      }
      else
      {
         if (noexc)
         {
            return false;
         }
         else
         {
            throw Error("Unsupported convertion for tag '%s'", mTag.c_str());
         }
      }
   }
   
   return true;
}

void Node::toStream(std::ostream &os, const std::string &indent, bool ignoreFirstIndent) const
{
   // Canonical form:
   //   tag:yaml.org,2002:null  -> "null"
   //   tag:yaml.org,2002:bool  -> "true" or "false"
   //   tag:yaml.org,2002:int   -> decimal value with possibliy leading '-'
   //   tag:yaml.org,2002:float -> ".inf", "-.inf", ".nan", floating point notation  "-? [1-9] ( \. [0-9]* [1-9] )? ( e [-+] [1-9] [0-9]* )?"
   //   tag:yaml.org,2002:float
   
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
      // properly handle +/-inf and nan?
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
   
   yaml::Node tmp;
   //tmp.setTag("bool");
   try
   {
      tmp.fromString("-198.876e-003");
      std::cout << "Value parsed: "; tmp.toStream(std::cout); std::cout << std::endl;
   }
   catch (yaml::Error &e)
   {
      std::cerr << "Could not parse value: " << e.what() << std::endl;
   }
   
   for (int i=1; i<argc; ++i)
   {
      yaml::Document doc;
      
      try
      {
         doc.read(argv[i]);
      }
      catch (yaml::ParserError &e)
      {
         std::cerr << "Failed to read YAML document '" << argv[i] << "'" << std::endl;
         std::cerr << e.what() << std::endl;
      }
      catch (...)
      {
         std::cerr << "Failed to read YAML document '" << argv[i] << "'" << std::endl;
         std::cerr << "  Unexpected error" << std::endl;
      }
   }
   
   return 0;
}
