/********************************************************************************
* Original Implementation by: Jeroen van der Zijp from: Fox-toolkit library     *
* Copyright (C) 1999,2005 by Jeroen van der Zijp.   All Rights Reserved.        *
********************************************************************************/

#ifndef __gcore_regexp_h_
#define __gcore_regexp_h_

#include <gcore/config.h>

// Ignore Escape Character Macro
#define IEC(str) ((const char*)gcore::_RawString(#str))

namespace gcore {
  
  enum RexError {
    REXERR_OK,
    REXERR_EMPTY,             // Empty pattern
    REXERR_PAREN,             // Unmatched parenthesis
    REXERR_BRACK,             // Unmatched bracket
    REXERR_BRACE,             // Unmatched brace
    REXERR_RANGE,             // Bad character range
    REXERR_ESC,               // Bad escape sequence
    REXERR_COUNT,             // Bad counted repeat
    REXERR_NOATOM,            // No atom preceding repetition
    REXERR_REPEAT,            // Repeat following repeat
    REXERR_BACKREF,           // Bad backward reference
    REXERR_CLASS,             // Bad character class
    REXERR_COMPLEX,           // Expression too complex
    REXERR_MEMORY,            // Out of memory
    REXERR_TOKEN              // Illegal token
  };
  
  // Regular expression parse flags
  enum {
    REX_NORMAL    = 0,        // Normal mode
    REX_CAPTURE   = 1,        // Perform capturing parentheses
    REX_ICASE     = 2,        // Case independent matching
    REX_NEWLINE   = 4,        // Match-any operators match newline too
    REX_VERBATIM  = 8,        // Disable interpretation of magic characters
    REX_SYNTAX    = 16        // Perform syntax check only
  };
  
  // Regular expression match flags
  enum {
    REX_FORWARD   = 0,        // Match scanning forward from offset
    REX_BACKWARD  = 32,       // Match scanning backward from offset
    REX_NOT_BOL   = 64,       // Start of string is NOT begin of line
    REX_NOT_EOL   = 128,      // End of string is NOT end of line
    REX_NOT_EMPTY = 256       // Do not match empty
  };
  
  struct GCORE_API _RawString {
    _RawString(const char *s)
      : e(s) {
      e.erase(0,1);
      e.erase(e.length()-1,1);
    }
    operator const char* () const {
      return e.c_str();
    }
    std::string e;
  };
  
  // Why IEC and _RawString ? -->
  // Regexp("(\\d\\d):(\\d\\d)") --> Regexp(IEC("(\d\d):(\d\d)"))
  
  class GCORE_API Regexp {
    public:
      
      class GCORE_API Match {
        
        public:
            
          friend class Regexp;
          
          Match(int mode=REX_FORWARD, int from=0, int to=2147483647);
          Match(const Match &rhs);
          ~Match();
          
          Match& operator=(const Match &rhs);
          
          // <0 value don't modify previous value
          void setRange(int from, int to);
          void setMode(int mode);
          
          std::string group(int i) const;
          std::string pre() const;
          std::string post() const;
          
        private:
            
          std::string  mStr;
          int          mFrom;
          int          mTo;
          int          mMode;
          int          mBeg[10];
          int          mEnd[10];
      };
      
    public:
      // Construct empty regular expression object
      Regexp();
      
      // Copy regular expression object
      Regexp(const Regexp& orig);
      
      // Compile expression from pattern; if error is not NULL,
      // error code is returned
      Regexp(const char* pattern, int mode=REX_NORMAL, RexError* error=0);
      
      // Compile expression from pattern; if error is not NULL,
      // error code is returned
      Regexp(const std::string& pattern, int mode=REX_NORMAL, RexError* error=0);
      
      // Delete
      ~Regexp();
      
      // Assign another regular expression to this one
      Regexp& operator=(const Regexp& orig);
      
      /*
       * See if regular expression is empty; the regular expression
       * will be empty when it is unable to parse a pattern due to
       * a syntax error.
       */
      bool empty() const;
      
      // Parse pattern, return error code if syntax error is found
      RexError parse(const char* pattern, int mode=REX_NORMAL);
      
      // Parse pattern, return error code if syntax error is found
      RexError parse(const std::string& pattern, int mode=REX_NORMAL);
      
      /*
       * Match a subject string of length len, returning TRUE if a match is found
       * and FALSE otherwise.  The entire pattern is captured in beg[0] and end[0],
       * where beg[0] refers to the position of the first matched character and
       * end[0] refers to the position after the last matched character.
       * Sub expressions from capturing parenthesis i are returned in beg[i] and
       * end[i].
       */
      bool match(
        const char* str, int len,
        int* beg = 0, int* end = 0,
        int mode = REX_FORWARD, int npar = 1,
        int fm = 0, int to = 2147483647) const;
      
      // Search for match in a string
      bool match(
        const std::string& str,
        int* beg = 0, int* end = 0,
        int mode = REX_FORWARD, int npar = 1,
        int fm = 0, int to = 2147483647) const;
        
      bool match(
        const std::string &str,
        Match &match) const;
      
      /*
       * After performing a regular expression match with capturing parentheses,
       * a substitution string is build from the replace string, where where "&" or "\0"
       * is replaced by the entire matched pattern, and "\1" through "\9" are
       * replaced by captured expressions. "\`" will be replaced by the part of the string preceding
       * the match, and "\'" by the remainder of the string.
       * The original source string and its length, and the match arrays beg and end must be passed.
       */
      static std::string substitute(
        const char* str, int len,
        int* beg, int* end, const std::string& replace, int npar=1);
      
      // Return substitution string
      static std::string substitute(
        const std::string& str, int* beg, int* end,
        const std::string& replace, int npar=1);
      
      static std::string substitute(
        Match &match,
        const std::string &replace);
      
      // Returns error code for given error
      static const char* getError(RexError err);
      
      // Comparison operators
      friend bool operator==(const Regexp &r1, const Regexp &r2);
      friend bool operator!=(const Regexp &r1, const Regexp &r2);
      
    private:
    
      int *code;
  
    private:
    
      static const char *const errors[];
      static const int fallback[];
  };

}

#endif



