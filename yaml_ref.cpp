#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <deque>

// ---

enum BOM
{
   UTF8 = 0,
   UTF16,
   UTF16_be = UTF16,
   UTF16_le,
   UTF32,
   UTF32_be = UTF32,
   UTF32_le
};

enum Chomping
{
   Strip = 0,
   Clip,
   Keep
};

enum Context
{
   Undefined = 0,
   BlockIn,
   BlockOut,
   FlowIn,
   FlowOut
   // BlockKey?
   // FlowKey?
};

class Char
{
private:
   union Value
   {
      char s;
      unsigned char u;
   };
public:
   Char()
   {
      mValue.s = '\0';
   }
   Char(char c)
   {
      mValue.s = c;
   }
   Char(unsigned char u)
   {
      mValue.u = u;
   }
   Char& operator=(char c)
   {
      mValue.s = c;
      return *this;
   }
   Char& operator=(unsigned char u)
   {
      mValue.u = u;
      return *this;
   }
   operator int () const
   {
      return int(mValue.s);
   }
private:
   Value mValue;
};

struct State
{
   Chomping chomp;
   Context ctx;
   int indent;
   
   State()
      : chomp(Strip)
      , ctx(Undefined)
      , indent(-1)
   {
   }
};

typedef std::deque<State> StateQueue;

struct Parser
{
   std::ifstream &in;
   StateQueue stack;
   State *state;
   BOM bom;
   // consume flag?
   
   Parser(std::ifstream &_in)
      : in(_in)
      , state(0)
      , bom(UTF8)
   {
      stack.push_back(State());
      state = &(stack.back());
   }
   
   bool next(Char &n)
   {
      if (in.good())
      {
         char c = '\0';
         in.get(c);
         if (in.good())
         {
            n = c;
            return true;
         }
      }
      return false;
   }
   
   bool prev(size_t n=1)
   {
      if (in.good())
      {
         for (size_t i=0; i<n; ++i)
         {
            in.unget();
            if (!in.good())
            {
               return false;
            }
         }
         return true;
      }
      return false;
   }
   
   unsigned short u16(Char c0, Char c1)
   {
      if (bom == UTF16_be || bom == UTF32_be)
      {
         // big endian
         // 0x0201
         // => 0x02, 0x01
         // => c0,   c1
         return (c0 << 8 | c1);
      }
      else
      {
         // little endian
         // 0x0201
         // => 0x01, 0x02
         // => c0,   c1
         return (c1 << 8 | c0);
      }
   }
   
   unsigned int u32(Char c0, Char c1, Char c2, Char c3)
   {
      if (bom == UTF16_be || bom == UTF32_be)
      {
         // big endian
         // 0x04030201
         // => 0x04, 0x03, 0x02, 0x01
         // => c0,   c1,   c2,   c3
         return (c0 << 24 | c1 << 16 | c2 << 8 | c3);
      }
      else
      {
         // little endian
         // 0x04030201
         // => 0x01, 0x02, 0x03, 0x04
         // => c0,   c1,   c2,   c3
         return (c3 << 24 | c2 << 16 | c1 << 8 | c0);
      }
   }
};

class StreamState
{
public:
   StreamState(Parser &parser)
      : mIS(parser.in)
      , mRestore(true)
   {
      mPos = mIS.tellg();
   }
   StreamState(std::istream &is)
      : mIS(is)
      , mRestore(true)
   {
      mPos = mIS.tellg();
   }
   ~StreamState()
   {
      if (mRestore)
      {
         mIS.seekg(mPos, mIS.beg);
      }
   }
   void consume()
   {
      mRestore = false;
   }
   void restore()
   {
      mIS.seekg(mPos, mIS.beg);
   }
private:
   StreamState();
   StreamState(const StreamState&);
   StreamState& operator=(const StreamState&);
private:
   std::istream &mIS;
   std::streampos mPos;
   bool mRestore;
};

// ---

bool c_printable(Parser &parser)
{
   StreamState _ss(parser);
   Char c0, c1, c2, c3;
   if (parser.next(c0))
   {
      if (c0 == 0x9 ||
          c0 == 0xA ||
          c0 == 0xD || 
          (0x20 <= c0 && c0 <= 0x7E))
      {
         // 8 bits
         _ss.consume();
         return true;
      }
      else if (parser.bom >= UTF16 && parser.next(c1))
      {
         unsigned short u16 = parser.u16(c0, c1);
         if ((0x00A0 <= u16 && u16 <= 0xD7FF) ||
             (0xE000 <= u16 && u16 <= 0xFFFD))
         {
            // 16 bits
            _ss.consume();
            return true;
         }
         else if (parser.bom >= UTF32 && parser.next(c2) && parser.next(c3))
         {
            unsigned int u32 = parser.u32(c0, c1, c2, c3); 
            if (0x00010000 <= u32 && u32 <= 0x0010FFFF)
            {
               // 32 bits
               _ss.consume();
               return true;
            }
         }
      }
   }
   return false;
}

bool nb_json(Parser &parser)
{
   StreamState _ss(parser);
   Char c0, c1, c2, c3;
   if (parser.next(c0))
   {
      if (c0 == 0x9)
      {
         _ss.consume();
         return true;
      }
      else
      {
         // 0x00000020 -> 0x0010FFFF
         Char c1, c2, c3;
         if (parser.next(c1) && parser.next(c2) && parser.next(c3))
         {
            // this actually depends on BOM
            unsigned int u32 = parser.u32(c0, c1, c2, c3);
            if (0x000020 <= u32 && u32 <= 0x10FFFF)
            {
               _ss.consume();
               return true;
            }
         }
      }
   }
   return false;
}

bool c_byte_order_mark(Parser &parser)
{
   StreamState _ss(parser);
   Char c0, c1, c2, c3;
   if (parser.next(c0))
   {
      if (c0 == 0x0)
      {
         if (parser.next(c1))
         {
            if (c1 == 0x0)
            {
               if (parser.next(c2) && parser.next(c3))
               {
                  if (c2 == 0xFE && c3 == 0xFF)
                  {
                     parser.bom = UTF32_be;
                     _ss.consume();
                     return true;
                  }
                  else if (c2 == 0x0)
                  {
                     parser.bom = UTF32_be;
                     _ss.consume();
                     return true;
                  }
               }
            }
            else
            {
               parser.bom = UTF16_be;
               _ss.consume();
               return true;
            }
         }
      }
      else if (c0 == 0xFF)
      {
         if (parser.next(c1))
         {
            if (c1 == 0xFE)
            {
               if (parser.next(c2))
               {
                  if (parser.next(c3))
                  {
                     if (c2 == 0x0 && c3 == 0x0)
                     {
                        parser.bom = UTF32_le;
                        _ss.consume();
                        return true;
                     }
                     parser.prev();
                  }
                  parser.prev();
               }
               _ss.consume();
               return true;
            }
         }
      }
      else if (c0 == 0xFE)
      {
         if (parser.next(c1))
         {
            if (c1 == 0xFF)
            {
               parser.bom = UTF16_be;
               _ss.consume();
            }
         }
      }
      else if (c0 == 0xEF)
      {
         if (parser.next(c1) && parser.next(c2))
         {
            if (c1 == 0xBB && c2 == 0xBF)
            {
               parser.bom = UTF8;
               _ss.consume();
               return true;
            }
         }
      }
      else
      {
         if (parser.next(c1))
         {
            if (c1 == 0x0)
            {
               if (parser.next(c2))
               {
                  if (parser.next(c3))
                  {
                     if (c2 == 0x0 && c3 == 0x0)
                     {
                        parser.bom = UTF32_le;
                        _ss.consume();
                        return true;
                     }
                     parser.prev();
                  }
                  parser.prev();
               }
               parser.bom = UTF16_le;
               _ss.consume();
               return true;
            }
         }
      }
   }
   return false;
}

bool c_sequence_entry(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && c == '-')
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool c_mapping_key(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && c == '?')
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool c_mapping_value(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && c == ':')
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool c_collect_entry(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && c == ',')
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool c_sequence_start(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && c == '[')
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool c_sequence_end(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && c == ']')
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool c_mapping_start(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && c == '{')
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool c_mapping_end(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && c == '}')
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool c_comment(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && c == '#')
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool c_anchor(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && c == '&')
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool c_alias(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && c == '*')
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool c_tag(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && c == '!')
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool c_literal(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && c == '|')
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool c_folder(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && c == '>')
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool c_single_quote(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && c == '\'')
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool c_double_quote(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && c == '"')
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool c_directive(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && c == '%')
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool c_reserved(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && (c == '@' || c == '`'))
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool c_indicator(Parser &parser)
{
   static const char *sIndicators = "-?:,[]{}#&*!|>'\"%@`";
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && strchr(sIndicators, c) != NULL)
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool c_flow_indicator(Parser &parser)
{
   static const char *sFlowIndicators = ",[]{}";
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && strchr(sFlowIndicators, c) != NULL)
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool b_line_feed(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && c == 0xA) // LF
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool b_carriage_return(Parser &parser)
{
   StreamState _ss(parser);
   Char c;
   if (parser.next(c) && c == 0xD) // CR
   {
      _ss.consume();
      return true;
   }
   return false;
}

bool b_char(Parser &parser)
{
   return (b_line_feed(parser) || b_carriage_return(parser));
}

bool nb_char(Parser &parser)
{
   StreamState _ss(parser);
   if (c_printable(parser))
   {
      // keep current pos
      std::streampos p = parser.in.tellg();
      
      // revert to orignal pos
      _ss.restore();
      
      if (b_char(parser) || c_byte_order_mark(parser))
      {
         return false;
      }
      
      parser.in.seekg(p, parser.in.beg);
      // tell StreamState to not restore stream position in destructor
      _ss.consume();
      return true;
   }
   return false;
}

bool b_break(Parser &parser)
{
   if (b_carriage_return(parser))
   {
      if (b_line_feed(parser))
      {
         // CR, LF
         return true;
      }
      else
      {
         // CR
         return true;
      }
   }
   else if (b_line_feed(parser))
   {
      // LF
      return true;
   }
   return false;
}

bool b_as_line_feed(Parser &parser)
{
   return b_break(parser);
}

bool b_non_content(Parser &parser)
{
   return b_break(parser);
}

// ---

int main(int argc, char **argv)
{
   for (int i=0; i<argc; ++i)
   {
      std::ifstream ifs(argv[i]);
      
      if (ifs.is_open())
      {
         Parser parser(ifs);
         
         // There we go!
      }
   }
   return 0;
}