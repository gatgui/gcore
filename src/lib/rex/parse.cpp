/*

Copyright (C) 2010  Gaetan Guidet

This file is part of gcore.

rex is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at
your option) any later version.

rex is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
USA.

*/

#include "parse.h"
#include "charclass.h"
#include "instruction.h"
#include <gcore/rex.h>

namespace gcore {

bool ReadDecimal(const char **ppc, unsigned long &v)
{
  const char *pc = *ppc;
  unsigned char digits[32];
  int i = 0;
  while (CHAR_IS(*pc,DIGIT_CHAR)) {
    digits[i++] = *pc - '0';
    ++pc;
  }
  if (i != 0) {
    unsigned long f = 1;
    v = 0;
    while (--i >= 0) {
      v += digits[i] * f;
      f *= 10;
    }
    *ppc = pc;
    return true;
  }
  return false;
}

bool ReadHexadecimal(const char **ppc, unsigned char &v)
{
  const char *pc = *ppc;
  unsigned char digits[2] = {0,0};
  int i = 0;
  while (CHAR_IS(*pc,HEXA_CHAR) && i<2) {
    if (CHAR_IS(*pc,DIGIT_CHAR)) {
      digits[i++] = *pc - '0';
    } else if (CHAR_IS(*pc,LOWER_CHAR)) {
      digits[i++] = 10 + (*pc - 'a');
    } else {
      digits[i++] = 10 + (*pc - 'A');
    }
    ++pc;
  }
  if (i != 0) {
    if (i == 1) {
      v = digits[0];
    } else {
      v = digits[0]*16 + digits[1];
    }
    *ppc = pc;
    return true;
  }
  return false;
}

bool ReadOctal(const char **ppc, unsigned char &v)
{
  const char *pc = *ppc;
  unsigned char digits[3] = {0,0,0};
  int i = 0;
  while (CHAR_IS(**ppc, DIGIT_CHAR) && i<3) {
    if (*pc > '7') return false;
    digits[i++] = *pc - '0';
    ++pc;
  }
  if (i != 0) {
    unsigned long lv;
    
    if (i == 1) {
      lv = digits[0];
    } else if (i == 2) {
      lv = digits[0]*8 + digits[1];
    } else {
      lv = digits[0]*64 + digits[1]*8 + digits[2];
    }
    
    if (lv >= 256) {
      return false;
    }
    v = (unsigned char)lv;
    *ppc = pc;
    return true;
  }
  return false;
}

// Expression ::= Branch { "|" Branch }*
Instruction* ParseExpression(const char **ppc, ParseInfo &info)
{
  Instruction *i0 = ParseBranch(ppc, info);
  while (**ppc == '|')
  {
    ++(*ppc);
    Instruction *i1 = ParseBranch(ppc, info);
    if (!i1)
    {
      delete i0;
      return 0;
    }
    Alternative *alt = new Alternative(i0, i1);
    i0 = alt;
  }
  return i0;
}

// Branch ::= { Piece }*
Instruction* ParseBranch(const char **ppc, ParseInfo &info)
{
  Instruction *first = 0;
  Instruction *last = 0;
  Instruction *i = 0;
  
  while (**ppc!='\0' && **ppc!='|' && **ppc!=')')
  {
    i = ParsePiece(ppc, info);
    if (!i)
    {
      if (first)
      {
        delete first;
      }
      return false;
    }
    if (!first)
    {
      first = i;
      last = i;
    }
    else
    {
      last->setNext(i);
      last = i;
    }
  }
  
  return first;
}

// Piece ::= Atom [ Repeat ]
Instruction* ParsePiece(const char **ppc, ParseInfo &info)
{
  Instruction *i = ParseAtom(ppc, info);
  if (i)
  {
    bool failed = false;
    Repeat *r = ParseRepeat(ppc, failed, info);
    if (r)
    {
      r->setInstruction(i);
      i = r;
    }
    else if (failed)
    {
      delete i;
      return 0;
    }
  }
  return i;
}

// Atom ::= "(" Expression ")" | "[" [^] Range "]" | Characters
Instruction* ParseAtom(const char **ppc, ParseInfo &info)
{
  Instruction *i = 0;
  const char *pc = *ppc;
  
  switch (*pc)
  {
    case '(':
    {
      Group::TriState dnl = Group::Inherit;
      Group::TriState nc = Group::Inherit;
      Group::TriState ml = Group::Inherit;
      bool capture = true;
      bool consume = true;
      bool invert = false;
      bool reverse = false;
      std::string name = "";
      
      ++pc;
      
      if (*pc == '?')
      {
        ++pc;
        if (*pc == '#')
        {
          // skip everything until ) and from behave as if ParseAtom was
          // called starting next character
          ++pc;
          while (*pc != ')')
          {
            if (*pc == '\0')
            {
              return 0;
            }
            ++pc;
          }
          *ppc = ++pc;
          return ParseAtom(ppc, info);
        }
        if (*pc == ':')
        {
          capture = false;
          ++pc;
        }
        else if (*pc == 'P')
        {
          ++pc;
          if (*pc == '<')
          {
            ++pc;
            while (*pc != '>')
            {
              if (*pc == '\0')
              {
                return 0;
              }
              name.push_back(*pc);
              ++pc;
            }
            ++pc;
          }
          else if (*pc == '=')
          {
            std::string name;
            ++pc;
            while (*pc != ')')
            {
              if (*pc == '\0')
              {
                return 0;
              }
              name.push_back(*pc);
              ++pc;
            }
            ++pc;
            *ppc = pc;
            return new Backsubst(name);
          }
          else
          {
            return 0;
          }
        }
        else if (*pc == '=')
        {
          // positive lookahead
          capture = false;
          consume = false;
          ++pc;
        }
        else if (*pc == '!')
        {
          // negative lookahead
          capture = false;
          consume = false;
          invert = true;
          ++pc;
        }
        else if (*pc == '<')
        {
          ++pc;
          if (*pc == '=')
          {
            // positive lookbehind
            capture = false;
            consume = false;
            reverse = true;
            ++pc;
          }
          else if (*pc == '!')
          {
            // negative lookbehind
            capture = false;
            consume = false;
            reverse = true;
            invert = true;
            ++pc;
          }
          else
          {
            std::cerr << "*** Invalid group format" << std::endl;
            return 0;
          }
        }
        else if (*pc == 'i' || *pc == 'm' || *pc == 's' || *pc == '-')
        {
          //capture = false;
          //consume = false;
          if (*pc == 'i')
          {
            // case sensitive off
            nc = Group::On;
            ++pc;
          }
          if (*pc == 'm')
          {
            // multiline on (. matches \r\n)
            ml = Group::On;
            ++pc;
          }
          if (*pc == 's')
          {
            // dot matches new line on
            dnl = Group::On;
            ++pc;
          }
          if (*pc == '-')
          {
            ++pc;
            if (*pc == 'i')
            {
              // case sensitive on
              nc = Group::Off;
              ++pc;
            }
            if (*pc == 'm')
            {
              // multiline off
              ml = Group::Off;
              ++pc;
            }
            if (*pc == 's')
            {
              // dot matches newline off
              dnl = Group::Off;
              ++pc;
            }
          }
          if (*pc != ':' && *pc != ')')
          {
            // either followed by : or group end (meaning we just want to change exp exec flags)
            std::cerr << "*** Invalid group format" << std::endl;
            return 0;
          }
          
          if (*pc == ':')
          {
            ++pc;
            // would having a closing parent here be problematic
          }
          else
          {
            capture = false;
            consume = false;
          }
        }
      }
      
      int gidx = (capture ? (++(info.numGroups)) : -1);
      unsigned short flags = (unsigned short)(reverse ? Rex::Reverse : 0);
      
      if (*pc != ')')
      {
        i = ParseExpression(&pc, info);
      }

      if (*pc != ')')
      {
        if (i)
        {
          delete i;
        }
        return 0;
      }
      
#ifdef _DEBUG
      std::cout << "Create group" << std::endl;
      std::cout << "  index: " << gidx << std::endl;
      std::cout << "  invert: " << invert << std::endl;
      std::cout << "  consume: " << consume << std::endl;
      std::cout << "  flags: " << flags << std::endl;
      std::cout << "  nc: " << nc << std::endl;
      std::cout << "  ml: " << ml << std::endl;
      std::cout << "  dnl: " << dnl << std::endl;
      std::cout << "  name: \"" << name << "\"" << std::endl;
      std::cout << "  code: " << std::endl;
      if (i)
      {
        i->toStream(std::cout, "    ");
      }
#endif
      
      i = new Group(gidx, i, !consume, invert, flags, nc, ml, dnl, name);
      
#ifdef _DEBUG
      std::cout << "Group created" << std::endl;
#endif
      ++pc;
      
      break;
    }
    case '[':
    {
      bool inv = false;
      ++pc;
      if (*pc == '^')
      {
        inv = true;
        ++pc;
      }
      i = ParseRange(&pc, inv, info);
      if (!i)
      {
        return 0;
      }
      if (*pc != ']')
      {
#ifdef _DEBUG
        std::cout << "Invalid character in expression: " << *pc << ", expected ]" << std::endl;
#endif
        if (i)
        {
          delete i;
        }
        return 0;
      }
      ++pc;
      break;
    }
    default:
      i = ParseCharacters(&pc, info);
      if (!i)
      {
        i = ParseZerowidth(&pc, info);
        if (!i)
        {
          return 0;
        }
      }
  }
  *ppc = pc;
  return i;
}

// Characters ::= { character }*
Instruction* ParseCharacters(const char **ppc, ParseInfo &)
{
  Instruction *i = 0;
  const char *pc = *ppc;
  unsigned char v;

  if (*pc!='\0' && !CHAR_IS(*pc, SPECIAL_CHAR))
  {
    if (*pc == '\\')
    {
      ++pc;
      switch (*pc)
      {
        // Predefined character class
        case 's': i = new Space(false); break;
        case 'S': i = new Space(true); break;
        case 'w': i = new Word(false); break;
        case 'W': i = new Word(true); break;
        case 'l': i = new Letter(false); break;
        case 'L': i = new Letter(true); break;
        case 'u': i = new UpperLetter(); break;
        case 'U': i = new LowerLetter(); break;
        case 'd': i = new Digit(false); break;
        case 'D': i = new Digit(true); break;
        case 'h': i = new Hexa(false); break;
        case 'H': i = new Hexa(true); break;
        // Escape sequence
        // i have a doubt on those ones in fact
        case 't': i = new Single('\t'); break;
        case 'v': i = new Single('\v'); break;
        case 'n': i = new Single('\n'); break;
        case 'r': i = new Single('\r'); break;
        case 'f': i = new Single('\f'); break;
        // This one is annoying as it gets caught before checking zerowidth assertion
        //case 'b': i = new Single('\b'); break;
        case 'a': i = new Single('\a'); break;
        //case 'e': i = new Single('\e'); break;
        // Control char
        case 'c': //control char: \cD
          ++pc;
          switch (*pc)
          {
            case '@': i = new Single('\x00'); break; // '\0'
            case 'A': i = new Single('\x01'); break;
            case 'B': i = new Single('\x02'); break;
            case 'C': i = new Single('\x03'); break;
            case 'D': i = new Single('\x04'); break;
            case 'E': i = new Single('\x05'); break;
            case 'F': i = new Single('\x06'); break;
            case 'G': i = new Single('\x07'); break; // '\a'
            case 'H': i = new Single('\x08'); break; // '\b'
            case 'I': i = new Single('\x09'); break; // '\t'
            case 'J': i = new Single('\x0A'); break; // '\n'
            case 'K': i = new Single('\x0B'); break; // '\v'
            case 'L': i = new Single('\x0C'); break; // '\f'
            case 'M': i = new Single('\x0D'); break; // '\r'
            case 'N': i = new Single('\x0E'); break;
            case 'O': i = new Single('\x0F'); break;
            case 'P': i = new Single('\x10'); break;
            case 'Q': i = new Single('\x11'); break;
            case 'R': i = new Single('\x12'); break;
            case 'S': i = new Single('\x13'); break;
            case 'T': i = new Single('\x14'); break;
            case 'U': i = new Single('\x15'); break;
            case 'V': i = new Single('\x16'); break;
            case 'W': i = new Single('\x17'); break;
            case 'X': i = new Single('\x18'); break;
            case 'Y': i = new Single('\x19'); break;
            case 'Z': i = new Single('\x1A'); break;
            case '[': i = new Single('\x1B'); break;
            case '\\': i = new Single('\x1C'); break;
            case ']': i = new Single('\x1D'); break;
            case '^': i = new Single('\x1E'); break;
            case '_': i = new Single('\x1F');
            default:  break;
          }
          break;
        // Hexa character
        case 'x':
          ++pc;
          if (ReadHexadecimal(&pc, v))
          {
            i = new Single(v);
          }
          break;
        // this will be an octal
        case '0':
          ++pc;
          if (ReadOctal(&pc, v))
          {
            i = new Single(v);
          }
          break;
        // Back substitution, or octal if not possible
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          v = *pc - '0';
          i = new Backsubst(v);
          break;
        // special char
        case '\\':
        case '(':
        case ')':
        case '{':
        case '}':
        case '[':
        case ']':
        case '*':
        case '+':
        case '?':
        case '.':
        case '^':
        case '$':
        case '|':
          i = new Single(*pc);
        default:
          // Not recognized !! [no zero width assertion for example]
          break;
      }
      if (!i)
      {
        return 0;
      }
    }
    else
    {
      i = new Single(*pc);
    }
    ++pc;
  }
  else if (*pc == '.')
  {
    i = new Any();
    ++pc;

  }
  else if (pc == *ppc)
  {
    return 0;
  }
  
  *ppc = pc;
  
  return i;
}

// Range ::= { character | character "-" character }+
Instruction* ParseRange(const char **ppc, bool inv, ParseInfo &)
{
  Instruction *first = 0;
  Instruction *last = 0;
  Instruction *i = 0;
  const char *pc = *ppc;
  unsigned char v;
  
  if (*pc == ']')
  {
    i = new Single(']');
    if (last)
    {
      last->setNext(i);
      last = i;
    }
    else
    {
      first = last = i;
    }
    ++pc;
  }
  
  if (*pc == '-')
  {
    i = new Single('-');
    if (last)
    {
      last->setNext(i);
      last = i;
    }
    else
    {
      first = last = i;
    }
    ++pc;
  }
  // - can also be juste before the ] char (situation dealt in the switch)
  
  bool hasPrev = false; // prev character was an escape sequence
  
  while (*pc != ']' && *pc != '\0')
  {
    i = 0;
    switch (*pc)
    {
      case '-':
      {
        unsigned char fc, lc;
        
        if (*(pc+1) == ']')
        {
          i = new Single('-');
          break;
        }
        
        if (hasPrev)
        {
          // we already have created a Single instruction for previous character
          // delete it, and replace by the new CharRange
          Instruction *p = last->prev();
          if (p)
          {
            p->setNext(0);
            last = p;
          }
          else
          {
            // last is the only instruction in list
            delete last;
            first = last = 0;
          }
          
          fc = *(pc-1);
          ++pc;
          
          switch(*pc)
          {
            case ']':
            case '\0':
            case '\\':
              break;
            default:
              lc = *pc;
              i = new CharRange(fc, lc);
              break;
          }
          hasPrev = false;
        }
        break;
      }
      case '\\':
      {
        ++pc;
        switch (*pc)
        {
          // Predefined character class
          case 's': i = new Space(false); break;
          case 'S': i = new Space(true); break;
          case 'w': i = new Word(false); break;
          case 'W': i = new Word(true); break;
          case 'l': i = new Letter(false); break;
          case 'L': i = new Letter(true); break;
          case 'u': i = new UpperLetter(); break;
          case 'U': i = new LowerLetter(); break;
          case 'd': i = new Digit(false); break;
          case 'D': i = new Digit(true); break;
          case 'h': i = new Hexa(false); break;
          case 'H': i = new Hexa(true); break;
          // Escape sequence
          case 't': i = new Single('\t'); break;
          case 'v': i = new Single('\v'); break;
          case 'n': i = new Single('\n'); break;
          case 'r': i = new Single('\r'); break;
          case 'f': i = new Single('\f'); break;
          case 'b': i = new Single('\b'); break;
          case 'a': i = new Single('\a'); break;
          //case 'e': i = new Single('\e'); break;
          case '\\': i = new Single('\\'); break;
          // Control char
          case 'c': //control char: \cD
            ++pc;
            switch (*pc)
            {
              case '@': i = new Single('\x00'); break;
              case 'A': i = new Single('\x01'); break;
              case 'B': i = new Single('\x02'); break;
              case 'C': i = new Single('\x03'); break;
              case 'D': i = new Single('\x04'); break;
              case 'E': i = new Single('\x05'); break;
              case 'F': i = new Single('\x06'); break;
              case 'G': i = new Single('\x07'); break; // '\a'
              case 'H': i = new Single('\x08'); break; // '\b'
              case 'I': i = new Single('\x09'); break; // '\t'
              case 'J': i = new Single('\x0A'); break; // '\n'
              case 'K': i = new Single('\x0B'); break; // '\v'
              case 'L': i = new Single('\x0C'); break; // '\f'
              case 'M': i = new Single('\x0D'); break; // '\r'
              case 'N': i = new Single('\x0E'); break;
              case 'O': i = new Single('\x0F'); break;
              case 'P': i = new Single('\x10'); break;
              case 'Q': i = new Single('\x11'); break;
              case 'R': i = new Single('\x12'); break;
              case 'S': i = new Single('\x13'); break;
              case 'T': i = new Single('\x14'); break;
              case 'U': i = new Single('\x15'); break;
              case 'V': i = new Single('\x16'); break;
              case 'W': i = new Single('\x17'); break;
              case 'X': i = new Single('\x18'); break;
              case 'Y': i = new Single('\x19'); break;
              case 'Z': i = new Single('\x1A'); break;
              case '[': i = new Single('\x1B'); break;
              case '\\': i = new Single('\x1C'); break;
              case ']': i = new Single('\x1D'); break;
              case '^': i = new Single('\x1E'); break;
              case '_': i = new Single('\x1F');
              default: break;
            }
            break;
          // Hexa character
          case 'x':
            ++pc;
            if (ReadHexadecimal(&pc, v))
            {
              i = new Single(v);
            }
            break;
          // Octal character [no back-substitute in char class]
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            ++pc;
            if (ReadOctal(&pc, v))
            {
              i = new Single(v);
            }
            break;
          case ']':
          case '^':
          case '-':
          case '*':
          case '+':
          case '?':
          case '{':
          case '}':
          case '(':
          case ')':
          case '$':
          case '[':
            i = new Single(*pc);
            ++pc;
          default:
            // Not recognized !! [no zero width assertion for example]
            break;
        }
        hasPrev = false;
        break;
      }
      default:
        i = new Single(*pc);
        hasPrev = true;
        break;
    }
    
    // did we read an instruction?
    
    if (!i)
    {
      if (first)
      {
        delete first;
      }
      return 0;
    }
    
    // append instruction to list
    
    if (last)
    {
      last->setNext(i);
      last = i;
    }
    else
    {
      first = last = i;
    }
    
    ++pc;
  }
  
  if (first)
  {
    *ppc = pc;
    return new CharClass(first, inv);
  }
  return 0;
}

// Repeat ::= ( "*" | "+" | "?" | Counts ) [ "?" ]
// Counts ::= "{" digits ["," [ digits] ] "}"
Repeat* ParseRepeat(const char **ppc, bool &err, ParseInfo &)
{
  const char *pc = *ppc;
  
  int rmin = 0;
  int rmax = -1;
  bool lazy = false;
  
  switch(*pc)
  {
    case '*':
      ++pc;
      break;
    case '+':
      ++pc;
      rmin = 1;
      break;
    case '?':
      ++pc;
      rmax = 1;
      break;
    case '{':
    {
      unsigned long r;
      ++pc;
      if (ReadDecimal(&pc, r))
      {
        rmin = (int)r;
        if (*pc == '}')
        {
          // matched {m}
          ++pc;
          rmax = rmin;
          break;
        }
        if (*pc != ',')
        {
          err = true;
          return 0;
        }
        ++pc;
        if (*pc == '}')
        {
          // matched {m,}
          ++pc;
          break;
        }
        if (!ReadDecimal(&pc, r))
        {
          err = true;
          return 0;
        }
        if (*pc != '}')
        {
          err = true;
          return 0;
        }
        ++pc;
        rmax = (int)r; // matched {m,n}
      }
      else
      {
        if (*pc == '}')
        {
          // matched {}
          ++pc;
          break;
        }
        if (*pc != ',')
        {
          err = true;
          return 0;
        }
        ++pc;
        if (!ReadDecimal(&pc, r))
        {
          err = true;
          return 0;
        }
        if (*pc != '}')
        {
          err = true;
          return 0;
        }
        // matched {,m}
        ++pc;
        rmax = (int)r;
      }
      break;
    }
    default:
      err = false;
      return 0;
  }
  
  if (*pc == '?')
  {
    ++pc;
    lazy = true;
  }
  
  *ppc = pc;
  
  err = false;
  return new Repeat(0, rmin, rmax, lazy);
}

Instruction* ParseZerowidth(const char **ppc, ParseInfo &)
{
  Instruction *i = 0;
  const char *pc = *ppc;
  
  switch (*pc)
  {
    case '^':
      i = new LineStart();
      break;
    case '$':
      i = new LineEnd();
      break;
    case '\\':
      ++pc;
      switch (*pc)
      {
        case 'b': i = new WordBound(false); break;
        case 'B': i = new WordBound(true); break;
        case '<': i = new WordStart(); break;
        case '>': i = new WordEnd(); break;
        case 'A': i = new StrStart(); break;
        case 'Z': i = new StrEnd(); break;
        case 'z': i = new BufferEnd();
        default: break;
      }
    default: break;
  }
  
  if (i)
  {
    ++pc;
    *ppc = pc;
    return i;
  }
  else
  {
    return 0;
  }
}

}
