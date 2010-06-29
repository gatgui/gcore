#include <gcore/rexp.h>
#include <assert.h>
#include <stdio.h>

#ifdef _MSC_VER
// conversion from '__w64 int' to 'int' warning (-Wp64 compile option)
// conversion from 'size_t' to 'int' warning
// unreachable code warning
# pragma warning(disable: 4244 4267 4702)
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*
  Syntax:
  =======

      ^         Match begin of line [if at begin of pattern]
      $         Match end of line [if at end of pattern]
      .         Match any character
      |         Alternation
      ( ... )   Grouping sub pattern
      (?i ... ) Match sub pattern case insensitive
      (?I ... ) Match sub pattern case sensitive
      (?n ... ) Match sub pattern with newlines
      (?N ... ) Match sub pattern with no newlines
      (?: ... ) Non-capturing parentheses
      (?= ... ) Zero width positive lookahead
      (?! ... ) Zero width negative lookahead
      []        Character class

      *         Match 0 or more [greedy]
      +         Match 1 or more [greedy]
      ?         Match 0 or 1 [greedy]
      {}        Match 0 or more [greedy]
      {n}       Match n times [greedy]
      {,m}      Match no more than m times [greedy]
      {n,}      Match n or more [greedy]
      {n,m}     Match at least n but no more than m times [greedy]

      *?        Match 0 or more [lazy]
      +?        Match 1 or more [lazy]
      ??        Match 0 or 1 [lazy]
      {}?       Match 0 or more times [lazy]
      {n}?      Match n times [lazy]
      {,m}?     Match no more than m times [lazy]
      {n,}?     Match n or more [lazy]
      {n,m}?    Match at least n but no more than m times [lazy]

      \a        Alarm, bell
      \e        Escape character
      \t        Tab
      \f        Form feed
      \n        Newline
      \r        Return
      \v        Vertical tab
      \cx       Control character
      \033      Octal
      \x1b      Hex
      \w        Word character [a-zA-Z_0-9]
      \W        Non-word character
      \l        Letter [a-zA-Z]
      \L        Non-letter
      \s        Space
      \S        Non-space
      \d        Digit [0-9]
      \D        Non-digit
      \h        Hex digit [0-9a-fA-F]
      \H        Non-hex digit
      \b        Word boundary
      \B        Word interior
      \u        Single uppercase character
      \U        Single lowercase character
      \p        Punctuation (not including '_')
      \P        Non punctuation
      \<        Begin of word
      \>        End of word
      \A        Match only beginning of string
      \Z        Match only and end of string
      \1 ... \9 Back reference


  Grammar:
  ========

      exp        ::= branch { "|" branch }*
      branch     ::= { piece }*
      piece      ::= atom [ rep ]
      rep        ::= ( "*" | "+" | "?" | counts ) [ "?" ]
      counts     ::= "{" digits ["," [ digits] ] "}"
      atom       ::= "(" exp ")" | "[" [^] range "]" | characters
      range      ::= { character | character "-" character } +
      characters ::= { character }*
      digits     ::= { digit }*
*/


// As close to infinity as we're going to get; this seems big
// enough.  We can not make it MAX_INT as this may wrap around when
// added to something else!
#define ONEINDIG 1000000

// Number of capturing sub-expressions allowed
#define NSUBEXP  10

// Size of string buffer
#define MAXCHARS 512

// Set operations
#define EXCL(set,ch) (set[((unsigned char)(ch))>>5]&=~(1<<(((unsigned char)(ch))&31)))
#define INCL(set,ch) (set[((unsigned char)(ch))>>5]|=(1<<(((unsigned char)(ch))&31)))
#define ISIN(set,ch) (set[((unsigned char)(ch))>>5]&(1<<(((unsigned char)(ch))&31)))
#define CLEAR(set)   (set[0]=set[1]=set[2]=set[3]=set[4]=set[5]=set[6]=set[7]=0)

using namespace gcore;

/*******************************************************************************/

namespace {

// Opcodes of the engine
enum {
  OP_END           =   0,           // End of pattern reached
  OP_FAIL          =   1,           // Always fail
  OP_SUCCEED       =   2,           // Always succeed
  OP_LINE_BEG      =   3,           // Beginning of line
  OP_LINE_END      =   4,           // End of line
  OP_WORD_BEG      =   5,           // Beginning of word
  OP_WORD_END      =   6,           // End of word
  OP_WORD_BND      =   7,           // Word boundary
  OP_WORD_INT      =   8,           // Word interior
  OP_STR_BEG       =   9,           // Beginning of string
  OP_STR_END       =  10,           // End of string
  OP_ANY_OF        =  11,           // Any character in set
  OP_ANY_BUT       =  12,           // Any character not in set
  OP_ANY           =  13,           // Any character but no newline
  OP_ANY_NL        =  14,           // Any character including newline
  OP_SPACE         =  15,           // White space
  OP_SPACE_NL      =  16,           // White space including newline
  OP_NOT_SPACE     =  17,           // Non-white space
  OP_DIGIT         =  18,           // Digit
  OP_NOT_DIGIT     =  19,           // Non-digit
  OP_NOT_DIGIT_NL  =  20,           // Non-digit including newline
  OP_LETTER        =  21,           // Letter
  OP_NOT_LETTER    =  22,           // Non-letter
  OP_NOT_LETTER_NL =  23,           // Non-letter including newline
  OP_WORD          =  24,           // Word character
  OP_NOT_WORD      =  25,           // Non-word character
  OP_NOT_WORD_NL   =  26,           // Non-word character including newline
  OP_HEX           =  27,           // Hex digit
  OP_NOT_HEX       =  28,           // Non hex digit
  OP_NOT_HEX_NL    =  29,           // Non hex digit including newline
  OP_PUNCT         =  30,           // Punctuation
  OP_NOT_PUNCT     =  31,           // Non punctuation
  OP_NOT_PUNCT_NL  =  32,           // Non punctuation including newline
  OP_CHARS         =  33,           // Match literal string
  OP_CHARS_CI      =  34,           // Match literal string, case insensitive
  OP_CHAR          =  35,           // Single character
  OP_CHAR_CI       =  36,           // Single character, case insensitive
  OP_JUMP          =  37,           // Jump to another location
  OP_BRANCH        =  38,           // Branch: jump after trying following code
  OP_BRANCHREV     =  39,           // Branch: jump before trying following code
  OP_STAR          =  40,           // Greedy * (simple)
  OP_MIN_STAR      =  41,           // Lazy * (simple)
  OP_PLUS          =  42,           // Greedy + (simple)
  OP_MIN_PLUS      =  43,           // Lazy + (simple)
  OP_QUEST         =  44,           // Greedy ? (simple)
  OP_MIN_QUEST     =  45,           // Lazy ? (simple)
  OP_REP           =  46,           // Greedy counted repeat (simple)
  OP_MIN_REP       =  47,           // Lazy counted repeat (simple)
  OP_LOOK_NEG      =  48,           // Negative look ahead
  OP_LOOK_POS      =  49,           // Positive look ahead
  OP_UPPER         =  50,           // Match upper case
  OP_LOWER         =  51,           // Match lower case
  OP_SUB_BEG       =  52,           // Start of substring i
  OP_SUB_END       =  62,           // End of substring i
  OP_REF           =  72,           // Back reference to substring i
  OP_REF_CI        =  82,           // Match substring i case insensitive
  OP_ZERO          =  92,           // Zero count i
  OP_INCR          = 102,           // Increment count i
  OP_JUMPLT        = 112,           // Jump if count i less than value
  OP_JUMPGT        = 122            // JUmp if count i greater than value
};


// Flags
enum  {
  FLG_WORST  = 0,           // Worst case
  FLG_WIDTH  = 1,           // Matches >=1 character
  FLG_SIMPLE = 2            // Simple
};


// Structure used during matching
struct Execute {
  const char  *str;               // String
  const char  *str_beg;           // Begin of string
  const char  *str_end;           // End of string
  int         *sub_beg;           // Begin of substring i
  int         *sub_end;           // End of substring i
  const int   *code;              // Program code
  int          npar;              // Number of capturing parentheses
  int          count[10];         // Counters for counted repeats
  int          mode;              // Match mode

  // Attempt to match
  bool attempt(const char* str);

  // Match at current string position
  bool match(const int* prog);

  // Execute
  bool execute(const char* fm,const char* to);
};


// Structure used during compiling
struct Compile {
  const char  *pat;               // Pattern string pointer
  int         *code;              // Program code
  int         *pc;                // Program counter
  int          mode;              // Compile mode
  int          nbra;              // Number of counting braces
  int          npar;              // Number of capturing parentheses

  // Code generation
  int* append(int op);
  int* append(int op,int arg);
  int* append(int op,int arg1,int arg2);
  int* append(int op,int set[]);
  int* append(int op,int len,int *data);
  int* insert(int *ptr,int op);
  int* insert(int *ptr,int op,int arg);
  int* insert(int *ptr,int op,int arg1,int arg2);

  // Patch branches
  void patch(int *fm,int *to);

  // Parsing
  RexError compile(int& flags);
  RexError expression(int& flags);
  RexError verbatim(int& flags);
  RexError alternative(int& flags);
  RexError piece(int& flags);
  RexError atom(int& flags);
  RexError charset();
};


/*******************************************************************************/

#ifndef NDEBUG

// Dump program
void dump(int *prog) {
  int op,min,max,no,val;
  fprintf(stderr,"\n");
  fprintf(stderr,"Program:\n");
  fprintf(stderr,"%-10p SIZE %d\n",(void*)prog,*prog);
  prog++;
  while (1) {
    fprintf(stderr,"%-10p ",(void*)prog);
    op=*prog++;
    switch (op) {
      case OP_END:
      fprintf(stderr,"OP_END\n");
      goto x;
    case OP_FAIL:
      fprintf(stderr,"OP_FAIL\n");
      break;
    case OP_SUCCEED:
      fprintf(stderr,"OP_SUCCEED\n");
      break;
    case OP_LINE_BEG:
      fprintf(stderr,"OP_LINE_BEG\n");
      break;
    case OP_LINE_END:
      fprintf(stderr,"OP_LINE_END\n");
      break;
    case OP_WORD_BEG:
      fprintf(stderr,"OP_WORD_BEG\n");
      break;
    case OP_WORD_END:
      fprintf(stderr,"OP_WORD_END\n");
      break;
    case OP_WORD_BND:
      fprintf(stderr,"OP_WORD_BND\n");
      break;
    case OP_WORD_INT:
      fprintf(stderr,"OP_WORD_INT\n");
      break;
    case OP_STR_BEG:
      fprintf(stderr,"OP_STR_BEG\n");
      break;
    case OP_STR_END:
      fprintf(stderr,"OP_STR_END\n");
      break;
    case OP_ANY_OF:
      fprintf(stderr,"OP_ANY_OF   \"");
      for (no=0; no<256; no++) {
        if (ISIN(prog,no)) {
          if (' '<=no) {
            fprintf(stderr,"%c",no);
          } else {
            fprintf(stderr,"\\x%02x",no);
          }
        }
      }
      fprintf(stderr,"\"\n");
      prog+=8;
      break;
    case OP_ANY_BUT:
      fprintf(stderr,"OP_ANY_BUT  \"");
      for (no=0; no<256; no++) {
        if (ISIN(prog,no)) {
          if (' '<=no) {
            fprintf(stderr,"%c",no);
          } else {
            fprintf(stderr,"\\x%02x",no);
          }
        }
      }
      fprintf(stderr,"\"\n");
      prog+=8;
      break;
    case OP_ANY:
      fprintf(stderr,"OP_ANY\n");
      break;
    case OP_ANY_NL:
      fprintf(stderr,"OP_ANY_NL\n");
      break;
    case OP_SPACE:
      fprintf(stderr,"OP_SPACE\n");
      break;
    case OP_SPACE_NL:
      fprintf(stderr,"OP_SPACE_NL\n");
      break;
    case OP_NOT_SPACE:
      fprintf(stderr,"OP_NOT_SPACE\n");
      break;
    case OP_DIGIT:
      fprintf(stderr,"OP_DIGIT\n");
      break;
    case OP_NOT_DIGIT:
      fprintf(stderr,"OP_NOT_DIGIT\n");
      break;
    case OP_NOT_DIGIT_NL:
      fprintf(stderr,"OP_NOT_DIGIT_NL\n");
      break;
    case OP_LETTER:
      fprintf(stderr,"OP_LETTER\n");
      break;
    case OP_NOT_LETTER:
      fprintf(stderr,"OP_NOT_LETTER\n");
      break;
    case OP_NOT_LETTER_NL:
      fprintf(stderr,"OP_NOT_LETTER_NL\n");
      break;
    case OP_WORD:
      fprintf(stderr,"OP_WORD\n");
      break;
    case OP_NOT_WORD:
      fprintf(stderr,"OP_NOT_WORD\n");
      break;
    case OP_NOT_WORD_NL:
      fprintf(stderr,"OP_NOT_WORD_NL\n");
      break;
    case OP_HEX:
      fprintf(stderr,"OP_HEX\n");
      break;
    case OP_NOT_HEX:
      fprintf(stderr,"OP_NOT_HEX\n");
      break;
    case OP_NOT_HEX_NL:
      fprintf(stderr,"OP_NOT_HEX_NL\n");
      break;
    case OP_PUNCT:
      fprintf(stderr,"OP_PUNCT\n");
      break;
    case OP_NOT_PUNCT:
      fprintf(stderr,"OP_NOT_PUNCT\n");
      break;
    case OP_NOT_PUNCT_NL:
      fprintf(stderr,"OP_NOT_PUNCT_NL\n");
      break;
    case OP_UPPER:
      fprintf(stderr,"OP_UPPER\n");
      break;
    case OP_LOWER:
      fprintf(stderr,"OP_LOWER\n");
      break;
    case OP_CHARS:
      no=*prog++;
      fprintf(stderr,"OP_CHARS     %d,\"",no);
      while (no>0) {
        if (' '<=*prog) {
          fprintf(stderr,"%c",*prog);
        } else {
          fprintf(stderr,"\\x%02x",*prog);
        }
        prog++;
        no--;
      }
      fprintf(stderr,"\"\n");
      break;
    case OP_CHARS_CI:
      no=*prog++;
      fprintf(stderr,"OP_CHARS_CI  %d,\"",no);
      while (no>0) {
        if (' '<=*prog) {
          fprintf(stderr,"%c",*prog);
        } else {
          fprintf(stderr,"\\x%02x",*prog);
        }
        prog++;
        no--;
      }
      fprintf(stderr,"\"\n");
      break;
    case OP_CHAR:
      fprintf(stderr,"OP_CHAR      \"");
      if (' '<=*prog) {
        fprintf(stderr,"%c",*prog);
      } else {
        fprintf(stderr,"\\x%02x",*prog);
      }
      fprintf(stderr,"\"\n");
      prog++;
      break;
    case OP_CHAR_CI:
      fprintf(stderr,"OP_CHAR_CI   \"");
      if (' '<=*prog) {
        fprintf(stderr,"%c",*prog);
      } else {
        fprintf(stderr,"\\x%02x",*prog);
      }
      fprintf(stderr,"\"\n");
      prog++;
      break;
    case OP_JUMP:
      fprintf(stderr,"OP_JUMP      %-10p\n",(void*)(*prog ? prog+*prog : 0));
      prog++;
      break;
    case OP_BRANCH:
      fprintf(stderr,"OP_BRANCH    %-10p\n",(void*)(*prog ? prog+*prog : 0));
      prog++;
      break;
    case OP_BRANCHREV:
      fprintf(stderr,"OP_BRANCHREV %-10p\n",(void*)(*prog ? prog+*prog : 0));
      prog++;
      break;
    case OP_STAR:
      fprintf(stderr,"OP_STAR\n");
      break;
    case OP_MIN_STAR:
      fprintf(stderr,"OP_MIN_STAR\n");
      break;
    case OP_PLUS:
      fprintf(stderr,"OP_PLUS\n");
      break;
    case OP_MIN_PLUS:
      fprintf(stderr,"OP_MIN_PLUS\n");
      break;
    case OP_QUEST:
      fprintf(stderr,"OP_QUEST\n");
      break;
    case OP_MIN_QUEST:
      fprintf(stderr,"OP_MIN_QUEST\n");
      break;
    case OP_REP:
      min=*prog++;
      max=*prog++;
      fprintf(stderr,"OP_REP       {%d,%d}\n",min,max);
      break;
    case OP_MIN_REP:
      min=*prog++;
      max=*prog++;
      fprintf(stderr,"OP_MIN_REP   {%d,%d}\n",min,max);
      break;
    case OP_LOOK_NEG:
      fprintf(stderr,"OP_LOOK_NEG  %-10p\n",(void*)(*prog ? prog+*prog : 0));
      prog++;
      break;
    case OP_LOOK_POS:
      fprintf(stderr,"OP_LOOK_POS  %-10p\n",(void*)(*prog ? prog+*prog : 0));
      prog++;
      break;
    case OP_SUB_BEG+0:
    case OP_SUB_BEG+1:
    case OP_SUB_BEG+2:
    case OP_SUB_BEG+3:
    case OP_SUB_BEG+4:
    case OP_SUB_BEG+5:
    case OP_SUB_BEG+6:
    case OP_SUB_BEG+7:
    case OP_SUB_BEG+8:
    case OP_SUB_BEG+9:
      fprintf(stderr,"OP_SUB_BEG%d\n",op-OP_SUB_BEG);
      break;
      case OP_SUB_END+0:
    case OP_SUB_END+1:
    case OP_SUB_END+2:
    case OP_SUB_END+3:
    case OP_SUB_END+4:
    case OP_SUB_END+5:
    case OP_SUB_END+6:
    case OP_SUB_END+7:
    case OP_SUB_END+8:
    case OP_SUB_END+9:
      fprintf(stderr,"OP_SUB_END%d\n",op-OP_SUB_END);
      break;
    case OP_REF+0:
    case OP_REF+1:
    case OP_REF+2:
    case OP_REF+3:
    case OP_REF+4:
    case OP_REF+5:
    case OP_REF+6:
    case OP_REF+7:
    case OP_REF+8:
    case OP_REF+9:
      fprintf(stderr,"OP_REF%d\n",op-OP_REF);
      break;
    case OP_REF_CI+0:
    case OP_REF_CI+1:
    case OP_REF_CI+2:
    case OP_REF_CI+3:
    case OP_REF_CI+4:
    case OP_REF_CI+5:
    case OP_REF_CI+6:
    case OP_REF_CI+7:
    case OP_REF_CI+8:
    case OP_REF_CI+9:
      fprintf(stderr,"OP_REF_CI%d\n",op-OP_REF_CI);
      break;
    case OP_ZERO+0:
    case OP_ZERO+1:
    case OP_ZERO+2:
    case OP_ZERO+3:
    case OP_ZERO+4:
    case OP_ZERO+5:
    case OP_ZERO+6:
    case OP_ZERO+7:
    case OP_ZERO+8:
    case OP_ZERO+9:
      fprintf(stderr,"OP_ZERO%d\n",op-OP_ZERO);
      break;
    case OP_INCR+0:
    case OP_INCR+1:
    case OP_INCR+2:
    case OP_INCR+3:
    case OP_INCR+4:
    case OP_INCR+5:
    case OP_INCR+6:
    case OP_INCR+7:
    case OP_INCR+8:
    case OP_INCR+9:
      fprintf(stderr,"OP_INCR%d\n",op-OP_INCR);
      break;
    case OP_JUMPLT+0:
    case OP_JUMPLT+1:
    case OP_JUMPLT+2:
    case OP_JUMPLT+3:
    case OP_JUMPLT+4:
    case OP_JUMPLT+5:
    case OP_JUMPLT+6:
    case OP_JUMPLT+7:
    case OP_JUMPLT+8:
    case OP_JUMPLT+9:
      val=*prog++;
      fprintf(stderr,"OP_JUMPLT%d   %d,%-10p\n",op-OP_JUMPLT,val,(void*)(*prog ? prog+*prog : 0));
      prog++;
      break;
    case OP_JUMPGT+0:
    case OP_JUMPGT+1:
    case OP_JUMPGT+2:
    case OP_JUMPGT+3:
    case OP_JUMPGT+4:
    case OP_JUMPGT+5:
    case OP_JUMPGT+6:
    case OP_JUMPGT+7:
    case OP_JUMPGT+8:
    case OP_JUMPGT+9:
      val=*prog++;
      fprintf(stderr,"OP_JUMPGT%d   %d,%-10p\n",op-OP_JUMPGT,val,(void*)(*prog ? prog+*prog : 0));
      prog++;
      break;
    default:
      fprintf(stderr,"OP_%d: error\n",op);
      goto x;
    }
  }
x:fprintf(stderr,"end\n");
}

#endif


/*******************************************************************************/

// Compile members

// Parse hex escape code
int hex(const char*& pat) {
  register int ch,n,c;
  for(ch=0,n=2; isxdigit((unsigned char)*pat) && n; n--) {
    c=(unsigned char)toupper((unsigned char)*pat++);
    ch=(ch<<4)+(('A'<=c)?c-'A'+10:c-'0');
  }
  return ch;
}


// Parse octal escape code
int oct(const char*& pat) {
  register int ch,n;
  for(ch=0,n=3; '0'<=((unsigned char)*pat) && ((unsigned char)*pat)<='7' && n; n--) {
    ch=(ch<<3)+(*pat++-'0');
  }
  return ch;
}


// Compiler main
RexError Compile::compile(int& flags) {
  RexError err;
  if (*pat=='\0') return REXERR_EMPTY;
  if (mode&REX_VERBATIM) {
    err=verbatim(flags);
  } else {
    err=expression(flags);
  }
  if (err!=REXERR_OK) {
    return err;
  }
  if (*pat!='\0') {
    return REXERR_PAREN;
  }
  append(OP_END);
  return REXERR_OK;
}


// Parse without interpretation of magic characters
RexError Compile::verbatim(int& flags) {
  int buf[MAXCHARS],ch,len;
  flags=FLG_WIDTH;
  while (*pat!='\0') {
    len=0;
    do {
      ch=*pat++;
      if (mode&REX_ICASE) ch=tolower((unsigned char)ch);
      buf[len++]=ch;
    } while(*pat!='\0' && len<MAXCHARS);
    if (len==1) {
      flags|=FLG_SIMPLE;
      append((mode&REX_ICASE)?OP_CHAR_CI:OP_CHAR,buf[0]);
    } else {
      append((mode&REX_ICASE)?OP_CHARS_CI:OP_CHARS,len,buf);
    }
  }
  return REXERR_OK;
}


// Parse expression
RexError Compile::expression(int& flags) {
  RexError err;
  int *at,*jp,flg;
  flags=FLG_WIDTH;
  at=pc;
  jp=NULL;
  err=alternative(flg);
  if (err!=REXERR_OK) {
    return err;
  }
  if (!(flg&FLG_WIDTH)) {
    flags&=~FLG_WIDTH;
  }
  while (*pat=='|') {
    pat++;
    insert(at,OP_BRANCH,pc-at+3);
    append(OP_JUMP,jp?jp-pc-1:0);
    jp=pc-1;
    at=pc;
    err=alternative(flg);
    if (err!=REXERR_OK) {
      return err;
    }
    if (!(flg&FLG_WIDTH)) {
      flags&=~FLG_WIDTH;
    }
  }
  patch(jp,pc);
  return REXERR_OK;
}


// Parse branch
RexError Compile::alternative(int& flags) {
  RexError err;
  int flg;
  flags=FLG_WORST;
  while (*pat!='\0' && *pat!='|' && *pat!=')') {
    err=piece(flg);
    if (err != REXERR_OK) {
      return err;
    }
    flags |= flg;
  }
  return REXERR_OK;
}


// Parse piece
RexError Compile::piece(int& flags) {
  int ch,rep_min,rep_max,lazy,flg,*ptr;
  RexError err;
  ptr=pc;
  err=atom(flg);
  if (err!=REXERR_OK) {
    return err;
  }
  
  if ((ch=*pat)=='*' || ch=='+' || ch=='?' || ch=='{') {
    // Followed by repetition
    if (!(flg & FLG_WIDTH)) {
      // Repeats may not match empty
      return REXERR_NOATOM;
    }
    pat++;
    lazy=0;
    rep_min=1;
    rep_max=1;
    switch (ch) {
    case '*':                                           // Repeat 0-INF
      rep_min=0;
      rep_max=ONEINDIG;
      break;
    case '+':                                           // Repeat 1-INF
      rep_min=1;
      rep_max=ONEINDIG;
      break;
    case '?':                                           // Repeat 0-1
      rep_min=0;
      rep_max=1;
      break;
    case '{':                                           // Repeat n-m
      rep_min=0;
      rep_max=ONEINDIG;
      if (*pat!='}') {
        while (isdigit((unsigned char)*pat)) {
          rep_min=10*rep_min+(*pat-'0');
          pat++;
        }
        rep_max=rep_min;
        if (*pat==',') {
          pat++;
          rep_max=ONEINDIG;
          if (*pat!='}') {
            rep_max=0;
            while (isdigit((unsigned char)*pat)) {
              rep_max=10*rep_max+(*pat-'0');
              pat++;
            }
          }
        }
        if (rep_min>rep_max) {
          // Illegal range
          return REXERR_RANGE;
        }
        if (rep_min==0 && rep_max==0) {
          // Bad count
          return REXERR_COUNT;
        }
      }
      if (*pat!='}') {
        // Unmatched brace
        return REXERR_BRACE;
      }
      pat++;
      break;
    }
    
    if (*pat=='?') {
      // Lazy
      pat++;
      lazy=1;
    }
    if (rep_min==0) {
      // If zero repetitions are allowed, then may have no width
      flg&=~FLG_WIDTH;
    }

    // Handle only non-trivial cases
    if (!(rep_min==1 && rep_max==1)) {
      // For simple repeats we prefix the last operation
      if (flg&FLG_SIMPLE) {
        if (rep_min==0 && rep_max==ONEINDIG) {
          insert(ptr,OP_STAR+lazy);
        } else if (rep_min==1 && rep_max==ONEINDIG) {
          insert(ptr,OP_PLUS+lazy);
        } else if (rep_min==0 && rep_max==1) {
          insert(ptr,OP_QUEST+lazy);
        } else {
          insert(ptr,OP_REP+lazy,rep_min,rep_max);
        }
      } else {
        // For complex repeats we build loop constructs
        if (rep_min==0 && rep_max==ONEINDIG) {
          /*    ________
          **   |        \
          ** --B--(...)--J--+--                 (...){0,ONEINDIG}
          **    \___________|
          */
          insert(ptr,lazy?OP_BRANCHREV:OP_BRANCH,pc-ptr+3);
          append(OP_JUMP,ptr-pc-1);
        } else if (rep_min==1 && rep_max==ONEINDIG) {
          /*    ________
          **   |        \
          ** --+--(...)--B--                    (...){1,ONEINDIG}
          **
          */
          append(lazy?OP_BRANCH:OP_BRANCHREV,ptr-pc-1);
        } else if (rep_min==0 && rep_max==1) {
          /*
          **
          ** --B--(...)--+--                    (...){0,1}
          **    \________|
          */
          insert(ptr,lazy?OP_BRANCHREV:OP_BRANCH,pc-ptr+1);
        } else if (0<rep_min && rep_min==rep_max) {
          /*       ___________
          **      |           \
          ** --Z--+--(...)--I--L--              (...){n,n}
          **
          */
          if (nbra>=NSUBEXP) {
            return REXERR_COMPLEX;
          }
          insert(ptr,OP_ZERO+nbra);
          append(OP_INCR+nbra);
          append(OP_JUMPLT+nbra,rep_min,ptr-pc-1);
          nbra++;
        } else if (rep_min==0 && rep_max<ONEINDIG) {
          /*       ___________
          **      |           \
          ** --Z--B--(...)--I--L--+--           (...){0,n}
          **       \______________|
          */
          if (nbra>=NSUBEXP) {
            return REXERR_COMPLEX;
          }
          insert(ptr,OP_ZERO+nbra);
          insert(ptr+1,lazy?OP_BRANCHREV:OP_BRANCH,pc-ptr+4);
          append(OP_INCR+nbra);
          append(OP_JUMPLT+nbra,rep_max,ptr-pc-1);
          nbra++;
        } else if (0<rep_min && rep_max==ONEINDIG) {
          /*       ________________
          **      |   ___________  \
          **      |  |           \  \
          ** --Z--+--+--(...)--I--L--B--        (...){n,ONEINDIG}
          */
          if (nbra>=NSUBEXP) {
            return REXERR_COMPLEX;
          }
          insert(ptr,OP_ZERO+nbra);
          append(OP_INCR+nbra);
          append(OP_JUMPLT+nbra,rep_min,ptr-pc-1);
          append(lazy?OP_BRANCH:OP_BRANCHREV,ptr-pc);
          nbra++;
        } else {
          /*       ___________________
          **      |   ___________     \
          **      |  |           \     \
          ** --Z--+--+--(...)--I--L--G--B--+--  (...){n,m}
          **                          \____|
          */
          if (nbra>=NSUBEXP) {
            return REXERR_COMPLEX;
          }
          insert(ptr,OP_ZERO+nbra);
          append(OP_INCR+nbra);
          append(OP_JUMPLT+nbra,rep_min,ptr-pc-1);
          append(OP_JUMPGT+nbra,rep_max,3);
          append(lazy?OP_BRANCH:OP_BRANCHREV,ptr-pc);
          nbra++;
        }
      }
    }
  }
  flags=flg&FLG_WIDTH;
  return REXERR_OK;
}


// Parse atom
RexError Compile::atom(int& flags) {
  int buf[MAXCHARS],level,save,ch,len,flg,*ptr;
  const char *p;
  RexError err;
  flags=FLG_WORST;                                // Assume the worst
  switch (*pat) {
    case '(':                                     // Subexpression grouping
    pat++;
    if (*pat=='?') {
      pat++;
      ch=*pat++;
      if (ch==':') {
        // Non capturing parentheses
        err=expression(flg);
        if (err!=REXERR_OK) return err; // Propagate error
      } else if (ch=='=' || ch=='!') {
        // Positive or negative look ahead
        append((ch=='=')?OP_LOOK_POS:OP_LOOK_NEG);
        ptr=append(0);
        err=expression(flg);
        if (err!=REXERR_OK) return err; // Propagate error
        append(OP_SUCCEED);
        patch(ptr,pc); // If trailing context matches (fails), go here!
        flg=FLG_WORST; // Look ahead has no width!
      } else if(ch=='i' || ch=='I' || ch=='n' || ch=='N') {
        save=mode;                              // Save flags
        if (ch=='i') mode|=REX_ICASE;
        if (ch=='I') mode&=~REX_ICASE;
        if (ch=='n') mode|=REX_NEWLINE;
        if (ch=='N') mode&=~REX_NEWLINE;
        err=expression(flg);
        if (err!=REXERR_OK) return err;          // Propagate error
        mode=save;                              // Restore flags
      } else {
        return REXERR_TOKEN;
      }
    } else if(mode&REX_CAPTURE) { 
      // Capturing
      level=++npar;
      if (level>=NSUBEXP) return REXERR_COMPLEX; // Expression too complex
      append(OP_SUB_BEG+level);
      err=expression(flg);
      if (err!=REXERR_OK) return err;            // Propagate error
      append(OP_SUB_END+level);
    } else {
      // Normal
      err=expression(flg);
      if (err!=REXERR_OK) return err;            // Propagate error
    }
    if (*pat!=')') return REXERR_PAREN;          // Unmatched parenthesis
    pat++;
    flags=flg&~FLG_SIMPLE;
    break;
  case '.':                                     // Any character
    pat++;
    append((mode&REX_NEWLINE)?OP_ANY_NL:OP_ANY);
    flags=FLG_WIDTH|FLG_SIMPLE;
    break;
  case '^':                                     // Begin of line
    pat++;
    append(OP_LINE_BEG);
    break;
  case '$':                                     // End of line
    pat++;
    append(OP_LINE_END);
    break;
  case '*':                                     // No preceding atom
  case '+':
  case '?':
  case '{':
    return REXERR_NOATOM;
  case '\0':                                    // Technically, this can not happen!
  case '|':
  case ')':
    return REXERR_NOATOM;
  case '}':                                     // Unmatched brace
    return REXERR_BRACE;
  case '[':
    pat++;
    err=charset();
    if (err!=REXERR_OK) return err;              // Bad character class
    if (*pat!=']') return REXERR_BRACK;          // Unmatched bracket
    pat++;
    flags=FLG_WIDTH|FLG_SIMPLE;
    break;
  case ']':                                     // Unmatched bracket
    return REXERR_BRACK;
  case '\\': // Escape sequences which are NOT part of simple character-run
    ch=*(pat+1);
    switch (ch) {
    case '\0':                                // Unexpected pattern end
      return REXERR_NOATOM;
    case 'w':                                 // Word character
      append(OP_WORD);
      pat+=2;
      flags=FLG_WIDTH|FLG_SIMPLE;
      return REXERR_OK;
    case 'W':                                 // Non-word character
      append((mode&REX_NEWLINE)?OP_NOT_WORD_NL:OP_NOT_WORD);
      pat+=2;
      flags=FLG_WIDTH|FLG_SIMPLE;
      return REXERR_OK;
    case 's':                                 // Space
      append((mode&REX_NEWLINE)?OP_SPACE_NL:OP_SPACE);
      pat+=2;
      flags=FLG_WIDTH|FLG_SIMPLE;
      return REXERR_OK;
    case 'S':                                 // Non-space
      append(OP_NOT_SPACE);
      pat+=2;
      flags=FLG_WIDTH|FLG_SIMPLE;
      return REXERR_OK;
    case 'd':                                 // Digit
      append(OP_DIGIT);
      pat+=2;
      flags=FLG_WIDTH|FLG_SIMPLE;
      return REXERR_OK;
    case 'D':                                 // Non-digit
      append((mode&REX_NEWLINE)?OP_NOT_DIGIT_NL:OP_NOT_DIGIT);
      pat+=2;
      flags=FLG_WIDTH|FLG_SIMPLE;
      return REXERR_OK;
    case 'h':                                 // Hex digit
      append(OP_HEX);
      pat+=2;
      flags=FLG_WIDTH|FLG_SIMPLE;
      return REXERR_OK;
    case 'H':                                 // Non-hex digit
      append((mode&REX_NEWLINE)?OP_NOT_HEX_NL:OP_NOT_HEX);
      pat+=2;
      flags=FLG_WIDTH|FLG_SIMPLE;
      return REXERR_OK;
    case 'p':                                 // Punctuation
      append(OP_PUNCT);
      pat+=2;
      flags=FLG_WIDTH|FLG_SIMPLE;
      return REXERR_OK;
    case 'P':                                 // Non-punctuation
      append((mode&REX_NEWLINE)?OP_NOT_PUNCT_NL:OP_NOT_PUNCT);
      pat+=2;
      flags=FLG_WIDTH|FLG_SIMPLE;
      return REXERR_OK;
    case 'l':                                 // Letter
      append(OP_LETTER);
      pat+=2;
      flags=FLG_WIDTH|FLG_SIMPLE;
      return REXERR_OK;
    case 'L':                                 // Non-letter
      append((mode&REX_NEWLINE)?OP_NOT_LETTER_NL:OP_NOT_LETTER);
      pat+=2;
      flags=FLG_WIDTH|FLG_SIMPLE;
      return REXERR_OK;
    case 'u':                                 // Upper case
      append(OP_UPPER);
      pat+=2;
      flags=FLG_WIDTH|FLG_SIMPLE;
      return REXERR_OK;
    case 'U':                                 // Lower case
      append(OP_LOWER);
      pat+=2;
      flags=FLG_WIDTH|FLG_SIMPLE;
      return REXERR_OK;
    case 'b':                                 // Word boundary
      append(OP_WORD_BND);
      pat+=2;
      return REXERR_OK;
    case 'B':                                 // Word interior
      append(OP_WORD_INT);
      pat+=2;
      return REXERR_OK;
    case 'A':                                 // Match only beginning of string
      append(OP_STR_BEG);
      pat+=2;
      return REXERR_OK;
    case 'Z':                                 // Match only and end of string
      append(OP_STR_END);
      pat+=2;
      return REXERR_OK;
    case '<':                                 // Begin of word
      append(OP_WORD_BEG);
      pat+=2;
      return REXERR_OK;
    case '>':                                 // End of word
      append(OP_WORD_END);
      pat+=2;
      return REXERR_OK;
    case '1':                                 // Back reference to previously matched subexpression
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      if (!(mode&REX_CAPTURE)) return REXERR_BACKREF;  // Can't do backreferences
      level=ch-'0';
      if (level>npar) return REXERR_BACKREF;           // Back reference out of range
      append((mode&REX_ICASE)?(OP_REF_CI+level):(OP_REF+level));
      pat+=2;
      return REXERR_OK;
    }
    /*fall*/
  default:
    len=0;
    do {
      p=pat;                                    // In case we need to back up...
      ch=*pat;
      switch (ch) {
      case '^':                               // Bail out on magic characters
      case '$':
      case '.':
      case '(':
      case ')':
      case '[':
      case ']':
      case '|':
        goto x;
      case '\\':
        ch=*(pat+1);
        switch (ch) {
        case 'w':                           // Bail out on special matching constructs
        case 'W':
        case 's':
        case 'S':
        case 'd':
        case 'D':
        case 'h':
        case 'H':
        case 'p':
        case 'P':
        case 'l':
        case 'L':
        case 'u':
        case 'U':
        case 'b':
        case 'B':
        case 'A':
        case 'Z':
        case '<':
        case '>':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          goto x;
        case 'a':                           // Bell
          pat+=2;
          ch='\a';
          break;
        case 'e':                           // Escape
          pat+=2;
          ch='\033';
          break;
        case 'f':                           // Form feed
          pat+=2;
          ch='\f';
          break;
        case 'n':                           // Newline
          pat+=2;
          ch='\n';
          break;
        case 'r':                           // Return
          pat+=2;
          ch='\r';
          break;
        case 't':                           // Tab
          pat+=2;
          ch='\t';
          break;
        case 'v':                           // Vertical tab
          pat+=2;
          ch='\v';
          break;
        case 'c':                           // Control character
          pat+=2;
          ch=*pat++;
          if(ch=='\0') return REXERR_NOATOM;// Unexpected pattern end
          ch=toupper(ch)-'@';
          break;
        case '0':                           // Octal digit
          pat+=2;
          ch=oct(pat);
          if(ch>256) return REXERR_TOKEN;   // Characters should be 0..255
          break;
        case 'x':                           // Hex digit
          pat+=2;
          ch=hex(pat);
          if(ch>256) return REXERR_TOKEN;   // Characters should be 0..255
          break;
        case '\0':                          // Unexpected pattern end
          return REXERR_NOATOM;
        default:
          pat+=2;
          break;
        }
        break;
      case '\0':                              // Unexpected pattern end
        return REXERR_NOATOM;
      default:
        pat++;
        break;
      }

      // Make lower case?
      if (mode&REX_ICASE) ch=tolower((unsigned char)ch);

      // Add to buffer
      buf[len++]=ch;
    } while (*pat!='\0' && *pat!='*' && *pat!='+' && *pat!='?' && *pat!='{' && len<MAXCHARS);

    // Back up one character if followed by a repetition: aaa* is interpreted as (aa)a*
x:    if (1<len && (*pat=='*' || *pat=='+' || *pat=='?' || *pat=='{')) {
      pat=p;
      len--;
    }

    assert(1<=len);

    // Had at least 1 character
    flags=FLG_WIDTH;

    if (len==1) {
      // Simple only if length is 1
      flags|=FLG_SIMPLE;
      append((mode&REX_ICASE)?OP_CHAR_CI:OP_CHAR,buf[0]);
    } else {
      // Array of characters
      append((mode&REX_ICASE)?OP_CHARS_CI:OP_CHARS,len,buf);
    }
    break;
  }
  return REXERR_OK;
}


// TRUE if character is a word character
inline int isword(int ch) {
  return isalnum((unsigned char)ch) || ch=='_';
}


// TRUE if character is punctuation (delimiter) character
inline int isdelim(int ch) {
  return ispunct(ch) && ch!='_';
}


// Parse character class
RexError Compile::charset() {
  register int first,last,op,i;
  int set[8];
  CLEAR(set);
  first=-1;
  if (*pat=='^') {
    // Negated character class
    op=OP_ANY_BUT;
    pat++;
  } else {
    op=OP_ANY_OF;
  }
  if (*pat=='-' || *pat==']') goto in;             // '-' and ']' are literal at begin
  while (*pat!='\0' && *pat!=']') {
in: last=*pat++;
    if (last=='\\') {
      last=*pat++;
      switch (last) {
        case 'w':
        for(i=0; i<256; i++) {if(isword(i)) INCL(set,i); }
        first=-1;
        continue;
      case 'W':
        for(i=0; i<256; i++){ if(!isword(i)) INCL(set,i); }
        first=-1;
        continue;
      case 's':
        for(i=0; i<256; i++){ if(isspace(i)) INCL(set,i); }
        first=-1;
        continue;
      case 'S':
        for(i=0; i<256; i++){ if(!isspace(i)) INCL(set,i); }
        first=-1;
        continue;
      case 'd':
        for(i=0; i<256; i++){ if(isdigit(i)) INCL(set,i); }
        first=-1;
        continue;
      case 'D':
        for(i=0; i<256; i++){ if(!isdigit(i)) INCL(set,i); }
        first=-1;
        continue;
      case 'h':
        for(i=0; i<256; i++){ if(isxdigit(i)) INCL(set,i); }
        first=-1;
        continue;
      case 'H':
        for(i=0; i<256; i++){ if(!isxdigit(i)) INCL(set,i); }
        first=-1;
        continue;
      case 'p':
        for(i=0; i<256; i++){ if(isdelim(i)) INCL(set,i); }
        first=-1;
        continue;
      case 'P':
        for(i=0; i<256; i++){ if(!isdelim(i)) INCL(set,i); }
        first=-1;
        continue;
      case 'l':
        for(i=0; i<256; i++){ if(isalpha((unsigned char)i)) INCL(set,i); }
        first=-1;
        continue;
      case 'L':
        for(i=0; i<256; i++){ if(!isalpha(i)) INCL(set,i); }
        first=-1;
        continue;
      case 'u':
        for(i=0; i<256; i++){ if(isupper(i)) INCL(set,i); }
        first=-1;
        continue;
      case 'U':
        for(i=0; i<256; i++){ if(islower(i)) INCL(set,i); }
        first=-1;
        continue;
      case 'a':                             // Bell
        last='\a';
        break;
      case 'e':                             // Escape
        last='\033';
        break;
      case 'b':                             // Backspace
        last='\b';
        break;
      case 'f':                             // Form feed
        last='\f';
        break;
      case 'n':                             // Newline
        last='\n';
        break;
      case 'r':                             // Return
        last='\r';
        break;
      case 't':                             // Tab
        last='\t';
        break;
      case 'v':                             // Vertical tab
        last='\v';
        break;
      case 'c':                             // Control character
        last=*pat++;
        if(last=='\0') return REXERR_NOATOM;// Unexpected pattern end
        last=toupper(last)-'@';
        break;
      case '0':                             // Octal digit
        last=oct(pat);
        break;
      case 'x':                             // Hex digit
        last=hex(pat);
        break;
      case '\0':
        return REXERR_NOATOM;               // Unexpected pattern end
      }
    }
    if (first==-1) {
      if (mode&REX_ICASE) {
        INCL(set,tolower(last));
        INCL(set,toupper(last));
      } else {
        INCL(set,last);
      }
      if (*pat=='-' && *(pat+1)!='\0' && *(pat+1)!=']') {
        first=last;
        pat++;
      }
    } else {
      if (first>=last) {
        // Bad range
        return REXERR_RANGE;
      }
      if (mode&REX_ICASE) {
        for (i=first; i<=last; i++) {
          INCL(set,tolower(i));
          INCL(set,toupper(i));
        }
      } else {
        for (i=first; i<=last; i++) {
          INCL(set,i);
        }
      }
      first=-1;
    }
  }

  // Are we matching newlines
  if ((op==OP_ANY_BUT) && !(mode&REX_NEWLINE) && !ISIN(set,'\n')) {
    INCL(set,'\n');
  }

  // Emit opcode
  append(op,set);
  return REXERR_OK;
}


// Append opcode
int* Compile::append(int op) {
  register int *val=pc;
  if (code) {
    pc[0]=op;
  }
  pc++;
  return val;
}


// Append one-argument opcode
int* Compile::append(int op,int arg) {
  register int *val=pc;
  if (code) {
    pc[0]=op;
    pc[1]=arg;
  }
  pc+=2;
  return val;
}


// Append two-argument opcode
int* Compile::append(int op,int arg1,int arg2) {
  register int *val=pc;
  if (code) {
    pc[0]=op;
    pc[1]=arg1;
    pc[2]=arg2;
  }
  pc+=3;
  return val;
}


// Append character class opcode
int* Compile::append(int op,int set[]) {
  register int *val=pc;
  if (code) {
    pc[0]=op;
    pc[1]=set[0];
    pc[2]=set[1];
    pc[3]=set[2];
    pc[4]=set[3];
    pc[5]=set[4];
    pc[6]=set[5];
    pc[7]=set[6];
    pc[8]=set[7];
  }
  pc+=9;
  return val;
}


// Append character array
int* Compile::append(int op,int len,int *data) {
  register int *val=pc;
  if (code) {
    pc[0]=op;
    pc[1]=len;
    memcpy(pc+2,data,sizeof(int)*len);
  }
  pc+=len+2;
  return val;
}


// Insert opcode at ptr
int* Compile::insert(int *ptr,int op) {
  if (code) {
    memmove(ptr+1,ptr,sizeof(int)*(pc-ptr));
    ptr[0]=op;
  }
  pc+=1;
  return ptr;
}


// Insert one-argument opcode at ptr
int* Compile::insert(int *ptr,int op,int arg) {
  if (code) {
    memmove(ptr+2,ptr,sizeof(int)*(pc-ptr));
    ptr[0]=op;
    ptr[1]=arg;
  }
  pc+=2;
  return ptr;
}


// Insert two-argument opcode at ptr
int* Compile::insert(int *ptr,int op,int arg1,int arg2) {
  if (code) {
    memmove(ptr+3,ptr,sizeof(int)*(pc-ptr));
    ptr[0]=op;
    ptr[1]=arg1;
    ptr[2]=arg2;
  }
  pc+=3;
  return ptr;
}



// Patch linked set of branches or jumps
// Example:
//
//      Before:        After:
//      ==========================
//      0:  OP_JUMP    0:  OP_JUMP
//      1:  0          1:  9
//      2:  ....       2:  ....
//      3:  OP_JUMP    3:  OP_JUMP
//      4:  -3         4:  6
//      5:  ....       5:  ....
//      6:  ....       6:  ....
//      7:  OP_JUMP    7:  OP_JUMP
// fm-> 8:  -4         8:  2
//      9:  ....       9:  ....
// to->10:  ....      10:  ....
//
void Compile::patch(int *fm,int *to) {
  register int delta;
  if (code && fm) {
    do {
      delta=*fm;
      *fm=to-fm;
      fm+=delta;
    } while(delta);
  }
}


/*******************************************************************************/

// Execute members

// The workhorse
bool Execute::match(const int* prog) {
  register int no,keep,rep_min,rep_max,greed,op;
  register const char *save,*beg,*end;
  register char ch;
  for (;;) {
    op=*prog++;
    switch (op) {
    case OP_END:
      return TRUE;
    case OP_FAIL:           // Fail (sub) pattern
      return FALSE;
    case OP_SUCCEED:        // Succeed (sub) pattern
      return TRUE;
    case OP_JUMP:
      prog+=*prog;
      break;
    case OP_BRANCH:         // Jump after trying following code
      save=str;
      if (match(prog+1)) return TRUE;
      str=save;
      prog+=*prog;
      break;
    case OP_BRANCHREV:      // Jump before trying following code
      save=str;
      if (match(prog+*prog)) return TRUE;
      str=save;
      prog++;
      break;
    case OP_LINE_BEG:       // Must be at begin of line
      if ((str==str_beg && (mode&REX_NOT_BOL)) || (str_beg<str && *(str-1)!='\n')) return FALSE;
      break;
    case OP_LINE_END:       // Must be at end of line
      if ((str==str_end && (mode&REX_NOT_EOL)) || (str<str_end && *str!='\n')) return FALSE;
      break;
    case OP_WORD_BEG:       // Must be at begin of word
      if (str_beg<str && isword((unsigned char) *(str-1))) return FALSE;
      if (str_end<=str || !isword((unsigned char) *str)) return FALSE;
      break;
    case OP_WORD_END:       // Must be at end of word
      if (str<str_end && isword((unsigned char) *str)) return FALSE;
      if (str<=str_beg || !isword((unsigned char) *(str-1))) return FALSE;
      break;
    case OP_WORD_BND:       // Must be at word boundary
      if (!(((str==str_beg || !isword((unsigned char) *(str-1))) && (str<str_end && isword((unsigned char) *str))) || ((str==str_end || !isword((unsigned char) *str)) && (str_beg<str && isword((unsigned char) *(str-1)))))) return FALSE;
      break;
    case OP_WORD_INT:       // Must be inside a word
      if (str==str_beg || !isword((unsigned char) *(str-1))) return FALSE;
      if (str==str_end || !isword((unsigned char) *str)) return FALSE;
      break;
    case OP_STR_BEG:        // Must be at begin of entire string
      if (str!=str_beg) return FALSE;
      break;
    case OP_STR_END:        // Must be at end of entire string
      if (str!=str_end) return FALSE;
      break;
    case OP_ANY_OF:         // Match a character in a set
      if (str==str_end || !ISIN(prog,*str)) return FALSE;
      prog+=8;
      str++;
      break;
    case OP_ANY_BUT:        // Match a character NOT in a set
      if (str==str_end || ISIN(prog,*str)) return FALSE;
      prog+=8;
      str++;
      break;
    case OP_CHAR:           // Match single character
      if (str==str_end || *prog != *str) return FALSE;
      prog++;
      str++;
      break;
    case OP_CHAR_CI:        // Match single character, disregard case
      if (str==str_end || *prog != tolower((unsigned char)*str)) return FALSE;
      prog++;
      str++;
      break;
    case OP_CHARS:          // Match a run of 1 or more characters
      no=*prog++;
      if (str+no>str_end) return FALSE;
      do {
        if (*prog++ != (unsigned char)*str++) return FALSE;
      } while(--no);
      break;
    case OP_CHARS_CI:       // Match a run of 1 or more characters, disregard case
      no=*prog++;
      if (str+no>str_end) return FALSE;
      do {
        if (*prog++ != tolower((unsigned char)*str++)) return FALSE;
      }
      while(--no);
      break;
    case OP_SPACE:          // Match space
      if (str==str_end || *str=='\n' || !isspace((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_SPACE_NL:       // Match space including newline
      if (str==str_end || !isspace((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_NOT_SPACE:      // Match non-space
      if (str==str_end || isspace((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_DIGIT:          // Match a digit 0..9
      if (str==str_end || !isdigit((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_NOT_DIGIT:      // Match a non-digit
      if (str==str_end || *str=='\n' || isdigit((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_NOT_DIGIT_NL:   // Match a non-digit including newline
      if (str==str_end || isdigit((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_HEX:            // Match a hex digit 0..9A-Fa-f
      if (str==str_end || !isxdigit((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_NOT_HEX:        // Match a non-hex digit
      if (str==str_end || *str=='\n' || isxdigit((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_NOT_HEX_NL:     // Match a non-hex digit including newline
      if (str==str_end || isxdigit((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_PUNCT:          // Match a punctuation
      if (str==str_end || !isdelim((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_NOT_PUNCT:      // Match a non-punctuation
      if (str==str_end || *str=='\n' || isdelim((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_NOT_PUNCT_NL:   // Match a non-punctuation including newline
      if (str==str_end || isdelim((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_LETTER:         // Match a letter a..z, A..Z
      if (str==str_end || !isalpha((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_NOT_LETTER:     // Match a non-letter
      if (str==str_end || *str=='\n' || isalpha((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_NOT_LETTER_NL:  // Match a non-letter including newline
      if (str==str_end || isalpha((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_WORD:           // Match a word character a..z,A..Z,0..9,_
      if (str==str_end || !isword((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_NOT_WORD:       // Match a non-word character
      if (str==str_end || *str=='\n' || isword((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_NOT_WORD_NL:    // Match a non-word character including newline
      if(str==str_end || isword((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_UPPER:          // Match if uppercase
      if (str==str_end || !isupper((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_LOWER:          // Match if lowercase
      if (str==str_end || !islower((unsigned char) *str)) return FALSE;
      str++;
      break;
    case OP_ANY:            // Match any character
      if (str==str_end || *str=='\n') return FALSE;
      str++;
      break;
    case OP_ANY_NL:          // Matches any character including newline
      if (str==str_end) return FALSE;
      str++;
      break;
    case OP_PLUS:         // Simple repetitions of one character
      rep_min=1;
      rep_max=ONEINDIG;
      greed=1;
      goto rep;
    case OP_MIN_PLUS:
      rep_min=1;
      rep_max=ONEINDIG;
      greed=0;
      goto rep;
    case OP_QUEST:
      rep_min=0;
      rep_max=1;
      greed=1;
      goto rep;
    case OP_MIN_QUEST:
      rep_min=0;
      rep_max=1;
      greed=0;
      goto rep;
    case OP_REP:
      rep_min=*prog++;
      rep_max=*prog++;
      greed=1;
      goto rep;
    case OP_MIN_REP:
      rep_min=*prog++;
      rep_max=*prog++;
      greed=0;
      goto rep;
    case OP_MIN_STAR:
      rep_min=0;
      rep_max=ONEINDIG;
      greed=0;
      goto rep;
    case OP_STAR:         // Star is the most popular so make that the fall-through case...
      rep_min=0;
      rep_max=ONEINDIG;
      greed=1;

      // We need to match more characters than are available
rep:  if (str+rep_min>str_end) return FALSE;
      beg=str;
      end=beg+rep_max;
      if (end>str_end) end=str_end;
      save=beg;

      // Find out how much could be matched
      op=*prog++;
      switch (op) {
      case OP_CHAR:
        ch=*prog++;
        while (save<end && *save==ch) save++;
        break;
      case OP_CHAR_CI:
        ch=*prog++;
        while (save<end && tolower((unsigned char) *save)==ch && *save!='\n') save++;
        break;
      case OP_CHARS:
        ch=*++prog;
        while (save<end && *save==ch) save++;
        prog+=3;
        break;
      case OP_CHARS_CI:
        ch=*++prog;
        while (save<end && tolower((unsigned char) *save)==ch) save++;
        prog+=3;
        break;
      case OP_ANY_OF:
        while (save<end && ISIN(prog,*save)) save++;
        prog+=8;
        break;
      case OP_ANY_BUT:
        while (save<end && !ISIN(prog,*save)) save++;
        prog+=8;
        break;
      case OP_SPACE:
        while (save<end && *save!='\n' && isspace((unsigned char) *save)) save++;
        break;
      case OP_SPACE_NL:
        while (save<end && isspace((unsigned char) *save)) save++;
        break;
      case OP_NOT_SPACE:
        while (save<end && !isspace((unsigned char) *save)) save++;
        break;
      case OP_DIGIT:
        while (save<end && isdigit((unsigned char) *save)) save++;
        break;
      case OP_NOT_DIGIT:
        while (save<end && *save!='\n' && !isdigit((unsigned char) *save)) save++;
        break;
      case OP_NOT_DIGIT_NL:
        while (save<end && !isdigit((unsigned char) *save)) save++;
        break;
      case OP_HEX:
        while (save<end && isxdigit((unsigned char) *save)) save++;
        break;
      case OP_NOT_HEX:
        while (save<end && *save!='\n' && !isxdigit((unsigned char) *save)) save++;
        break;
      case OP_NOT_HEX_NL:
        while (save<end && !isxdigit((unsigned char) *save)) save++;
        break;
      case OP_PUNCT:
        while (save<end && isdelim((unsigned char) *save)) save++;
        break;
      case OP_NOT_PUNCT:
        while (save<end && *save!='\n' && !isdelim((unsigned char) *save)) save++;
        break;
      case OP_NOT_PUNCT_NL:
        while (save<end && !isdelim((unsigned char) *save)) save++;
        break;
      case OP_LETTER:
        while(save<end && isalpha((unsigned char) *save)) save++;
        break;
      case OP_NOT_LETTER:
        while (save<end && *save!='\n' && !isalpha((unsigned char) *save)) save++;
        break;
      case OP_NOT_LETTER_NL:
        while (save<end && !isalpha((unsigned char) *save)) save++;
        break;
      case OP_WORD:
        while (save<end && isword((unsigned char) *save)) save++;
        break;
      case OP_NOT_WORD:
        while (save<end && *save!='\n' && !isword((unsigned char) *save)) save++;
        break;
      case OP_NOT_WORD_NL:
        while (save<end && !isword((unsigned char) *save)) save++;
        break;
      case OP_UPPER:
        while (save<end && isupper((unsigned char) *save)) save++;
        break;
      case OP_LOWER:
        while (save<end && islower((unsigned char) *save)) save++;
        break;
      case OP_ANY:
        while (save<end && *save!='\n') save++;
        break;
      case OP_ANY_NL:
        save=end; // Big byte
        break;
      default:
        fprintf(stderr,
          "Regexp::match: bad opcode (%d) at: %p on line: %d\n",
          op, (void*)(prog-1), __LINE__);
        break;
      }

      // Matched fewer than the minimum desired so bail out
      if (save<beg+rep_min) return FALSE;

      // We must match between beg and end characters
      beg+=rep_min;
      end=save;

      // Greedily match the most characters
      if (greed) {
        while (beg<=end) {
          str=end;
          if (match(prog)) return TRUE;
          end--;
        }
      } else {
        // Lazily match the fewest characters
        while (beg<=end) {
          str=beg;
          if (match(prog)) return TRUE;
          beg++;
        }
      }
      return FALSE;
    case OP_SUB_BEG+0:              // Capturing open parentheses
    case OP_SUB_BEG+1:
    case OP_SUB_BEG+2:
    case OP_SUB_BEG+3:
    case OP_SUB_BEG+4:
    case OP_SUB_BEG+5:
    case OP_SUB_BEG+6:
    case OP_SUB_BEG+7:
    case OP_SUB_BEG+8:
    case OP_SUB_BEG+9:
      no=op-OP_SUB_BEG;
      if (no>=npar) break;           // Match w/o capture if array too small
      keep=sub_beg[no];             // Keep old value
      sub_beg[no]=str-str_beg;      // Tentatively set new value
      if (match(prog)) return TRUE;  // Match the rest
      sub_beg[no]=keep;             // Restore old value
      return FALSE;
    case OP_SUB_END+0:              // Capturing close parentheses
    case OP_SUB_END+1:
    case OP_SUB_END+2:
    case OP_SUB_END+3:
    case OP_SUB_END+4:
    case OP_SUB_END+5:
    case OP_SUB_END+6:
    case OP_SUB_END+7:
    case OP_SUB_END+8:
    case OP_SUB_END+9:
      no=op-OP_SUB_END;
      if (no>=npar) break;           // Match w/o capture if array too small
      keep=sub_end[no];
      sub_end[no]=str-str_beg;      // Remember capture end for future back reference
      if (match(prog)) return TRUE;
      sub_end[no]=keep;             // Restore old value
      return FALSE;
    case OP_REF+0:                  // Back reference to capturing parentheses
    case OP_REF+1:
    case OP_REF+2:
    case OP_REF+3:
    case OP_REF+4:
    case OP_REF+5:
    case OP_REF+6:
    case OP_REF+7:
    case OP_REF+8:
    case OP_REF+9:
      no=op-OP_REF;
      if (no>=npar) return FALSE;                    // Arrays were too small
      if (sub_beg[no]<0) return FALSE;               // Not captured yet
      if (sub_end[no]<0) return FALSE;               // Not captured yet
      beg=str_beg+sub_beg[no];
      end=str_beg+sub_end[no];
      if (beg<end) {
        // Empty capture matches!
        if (str+(end-beg)>str_end) return FALSE;     // Not enough characters left
        do {
          if (*beg != *str) return FALSE;            // No match
          beg++;
          str++;
        } while(beg<end);
      }
      break;
    case OP_REF_CI+0:               // Back reference to capturing parentheses
    case OP_REF_CI+1:
    case OP_REF_CI+2:
    case OP_REF_CI+3:
    case OP_REF_CI+4:
    case OP_REF_CI+5:
    case OP_REF_CI+6:
    case OP_REF_CI+7:
    case OP_REF_CI+8:
    case OP_REF_CI+9:
      no=op-OP_REF_CI;
      if (no>=npar) return FALSE;                    // Arrays were too small
      if (sub_beg[no]<0) return FALSE;               // Not captured yet
      if (sub_end[no]<0) return FALSE;               // Not captured yet
      beg=str_beg+sub_beg[no];
      end=str_beg+sub_end[no];
      if (beg<end) {
        // Empty capture matches!
        if (str+(end-beg)>str_end) return FALSE;     // Not enough characters left
        do {
          if (*beg != tolower((unsigned char)*str)) return FALSE;            // No match
          beg++;
          str++;
        } while(beg<end);
      }
      break;
    case OP_LOOK_NEG:               // Positive or negative look ahead
    case OP_LOOK_POS:
      save=str;
      keep=match(prog+1);
      str=save;
      if ((op-OP_LOOK_NEG)!=keep) return FALSE;      // Didn't get what we expected
      prog=prog+*prog;              // Jump to code after OP_SUCCEED
      break;
    case OP_ZERO+0:                 // Initialize counter for counting repeat
    case OP_ZERO+1:
    case OP_ZERO+2:
    case OP_ZERO+3:
    case OP_ZERO+4:
    case OP_ZERO+5:
    case OP_ZERO+6:
    case OP_ZERO+7:
    case OP_ZERO+8:
    case OP_ZERO+9:
      count[op-OP_ZERO]=0;
      break;
    case OP_INCR+0:                 // Increment counter for counting repeat
    case OP_INCR+1:
    case OP_INCR+2:
    case OP_INCR+3:
    case OP_INCR+4:
    case OP_INCR+5:
    case OP_INCR+6:
    case OP_INCR+7:
    case OP_INCR+8:
    case OP_INCR+9:
      count[op-OP_INCR]++;
      break;
    case OP_JUMPLT+0:               // Jump if counter less than value
    case OP_JUMPLT+1:
    case OP_JUMPLT+2:
    case OP_JUMPLT+3:
    case OP_JUMPLT+4:
    case OP_JUMPLT+5:
    case OP_JUMPLT+6:
    case OP_JUMPLT+7:
    case OP_JUMPLT+8:
    case OP_JUMPLT+9:
      if (count[op-OP_JUMPLT] < *prog++) {
        // Compare with value
        prog+=*prog;
      } else {
        prog++;
      }
      break;
    case OP_JUMPGT+0:               // Jump if counter greater than value
    case OP_JUMPGT+1:
    case OP_JUMPGT+2:
    case OP_JUMPGT+3:
    case OP_JUMPGT+4:
    case OP_JUMPGT+5:
    case OP_JUMPGT+6:
    case OP_JUMPGT+7:
    case OP_JUMPGT+8:
    case OP_JUMPGT+9:
      if (count[op-OP_JUMPGT] > *prog++) {
        // Compare with value
        prog+=*prog;
      } else {
        prog++;
      }
      break;
    default:
      fprintf(stderr,
        "Regexp::match: bad opcode (%d) at: %p on line: %d\n",
        op, (void*)(prog-1), __LINE__);
      break;
    }
  }
  return FALSE;
}


// regtry - try match at specific point; 0 failure, 1 success
bool Execute::attempt(const char* string) {
  register int i=npar;
  str=string;
  do {
    --i;
    sub_beg[i]=sub_end[i]=-1;
  } while (i); // Possibly move this to Execute::execute?
  if (match(code+1)) {
    if (string!=str || !(mode&REX_NOT_EMPTY)) {
      // Match if non-empty or empty is allowed!         
      sub_beg[0]=string-str_beg;
      sub_end[0]=str-str_beg;
      return TRUE;
    }
  }
  return FALSE;
}


// Match subject string, returning number of matches found
bool Execute::execute(const char* fm,const char* to) {
  register char ch;

  // Simple case
  if (fm==to) return attempt(fm);

  // Match backwards
  if (mode&REX_BACKWARD) {
    if (code[1]==OP_STR_BEG) {                          
      // Anchored at string start
      return (fm==str_beg) && attempt(str_beg);
    }
    if (code[1]==OP_LINE_BEG) {       
      // Anchored at BOL
      while (fm<=to) {
        if (((to==str_beg)||(*(to-1)=='\n')) && attempt(to)) return TRUE;
        to--;
      }
      return FALSE;
    }
    if (code[1]==OP_CHAR || code[1]==OP_CHARS) {        
      // Known starting character
      ch=(code[1]==OP_CHAR)?code[2]:code[3];
      if (to==str_end) to--;
      while (fm<=to) {
        if (*to==ch && attempt(to)) return TRUE;
        to--;
      }
      return FALSE;
    }
    if (code[1]==OP_CHAR_CI || code[1]==OP_CHARS_CI) {  
      // Known starting character, ignoring case
      ch=(code[1]==OP_CHAR_CI)?code[2]:code[3];
      if (to==str_end) to--;
      while (fm<=to) {
        if (tolower((unsigned char)*to)==ch && attempt(to)) return TRUE;
        to--;
      }
      return FALSE;
    }
    while(fm<=to) {
      // General case
      if (attempt(to)) return TRUE;
      to--;
    }
  } else {
    // Match forwards
    if (code[1]==OP_STR_BEG) {
      // Anchored at string start
      return (fm==str_beg) && attempt(str_beg);
    }
    if (code[1]==OP_LINE_BEG) {
      // Anchored at BOL
      while (fm<=to) {
        if (((fm==str_beg)||(*(fm-1)=='\n')) && attempt(fm)) return TRUE;
        fm++;
      }
      return FALSE;
    }
    if (code[1]==OP_CHAR || code[1]==OP_CHARS) {
      // Known starting character
      ch=(code[1]==OP_CHAR)?code[2]:code[3];
      if (to==str_end) to--;
      while (fm<=to) {
        if (*fm==ch && attempt(fm)) return TRUE;
        fm++;
      }
      return FALSE;
    }
    if (code[1]==OP_CHAR_CI || code[1]==OP_CHARS_CI) {
      // Known starting character, ignoring case
      ch=(code[1]==OP_CHAR_CI)?code[2]:code[3];
      if (to==str_end) {
        to--;
      }
      while (fm<=to) {
        if (tolower((unsigned char)*fm)==ch && attempt(fm)) {
          return TRUE;
        }
        fm++;
      }
      return FALSE;
    }
    while (fm<=to) {
      // General case
      if (attempt(fm)) {
        return TRUE;
      }
      fm++;
    }
  }
  return FALSE;
}

}

/*******************************************************************************/

namespace gcore {

// Table of error messages
const char *const Regexp::errors[]={
  "OK",
  "Empty pattern",
  "Unmatched parenthesis",
  "Unmatched bracket",
  "Unmatched brace",
  "Bad character range",
  "Bad escape sequence",
  "Bad counted repeat",
  "No atom preceding repetition",
  "Repeat following repeat",
  "Bad backward reference",
  "Bad character class",
  "Expression too complex",
  "Out of memory",
  "Illegal token"
  };


MatchData::MatchData(int dim, int mode, int from, int to)
  : mFrom(from), mTo(to), mMode(mode), mDim(dim), mCount(0) {
  if (mDim <= 0) {
    mDim = 1;
  }
  mBeg = new int[mDim];
  mEnd = new int[mDim];
}

MatchData::~MatchData() {
  delete[] mBeg;
  delete[] mEnd;
}

int MatchData::getCount() const {
  return mCount;
}

void MatchData::setRange(int from, int to) {
  if (from >= 0) {
    mFrom = from;
  }
  if (to >= 0) {
    mTo = to;
  }
}

void MatchData::setMode(int mode) {
  mMode = mode;
}

void MatchData::setDim(int dim) {
  mDim = dim;
  if (mDim <= 0) {
    mDim = 1;
  }
  delete[] mBeg;
  delete[] mEnd;
  mBeg = new int[mDim];
  mEnd = new int[mDim];
}

const std::string MatchData::get(const std::string &str, int i) const {
  if (i>=0 && i<mDim && mBeg[i]!=-1 && mEnd[i]!=-1) {
    return str.substr(mBeg[i], mEnd[i]-mBeg[i]);
  } else {
    return "";
  }
}

const std::string MatchData::getPre(const std::string &str) const {
  if (mBeg[0]!=-1 && mEnd[0]!=-1) {
    return str.substr(0, mBeg[0]);
  } else {
    return "";
  }
}

const std::string MatchData::getPost(const std::string &str) const {
  if (mBeg[0]!=-1 && mEnd[0]!=-1) {
    return str.substr(mEnd[0]);
  } else {
    return "";
  }
}

// Default program always fails
const int Regexp::fallback[]={2,OP_FAIL};


// Copy regex object
Regexp::Regexp(const Regexp& orig) {
  code = (int*)fallback;
  if (orig.code != fallback) {
    code = (int*)malloc(orig.code[0] * sizeof(int));
    memcpy(code, orig.code, orig.code[0] * sizeof(int));
  }
}


// Compile expression from pattern; fail if error
Regexp::Regexp(const char* pattern,int mode,RexError* error):code((int*)fallback) {
  RexError err=parse(pattern,mode);
  if (error) {
    *error=err;
  }
}


// Compile expression from pattern; fail if error
Regexp::Regexp(const std::string& pattern,int mode,RexError* error):code((int*)fallback) {
  RexError err=parse(pattern.c_str(),mode);
  if (error) {
    *error=err;
  }
}


// Assignment
Regexp& Regexp::operator=(const Regexp& orig) {
  if (code!=orig.code) {
    if (code!=fallback) {
      free(code);
      code = 0;
    }
    code=(int*)fallback;
    if (orig.code!=fallback) {
      code = (int*)malloc(orig.code[0]*sizeof(int));
      memcpy(code, orig.code, orig.code[0]*sizeof(int));
    }
  }
  return *this;
}


// Parse pattern
RexError Regexp::parse(const char* pattern,int mode) {
  RexError err=REXERR_EMPTY;
  Compile cs;
  int flags,size;

  // Free old code, if any
  if (code!=fallback) {
    free(code);
    code = 0;
  }
  code=(int*)fallback;

  // Check
  if (pattern) {

    // Fill in compile data
    cs.code=NULL;
    cs.pc=NULL;
    cs.pat=pattern;
    cs.mode=mode;
    cs.nbra=0;
    cs.npar=0;

    // Unknown size
    cs.append(0);

    // Check syntax and amount of memory needed
    err=cs.compile(flags);
    if (err==REXERR_OK) {

      // Compile code unless only syntax checking
      if (!(mode&REX_SYNTAX)) {

        // Allocate new code
        size=cs.pc-((int*)NULL);
        code = (int*)malloc(size*sizeof(code));
        if (!code) {
          code=(int*)fallback;
          return REXERR_MEMORY;
        }

        // Fill in compile data
        cs.code=code;
        cs.pc=code;
        cs.pat=pattern;
        cs.mode=mode;
        cs.nbra=0;
        cs.npar=0;

        // Size of program
        cs.append(size);

        // Generate program
        err=cs.compile(flags);
      }
    }
  }
  return err;
}


// Parse pattern, return error code if syntax error is found
RexError Regexp::parse(const std::string& pattern,int mode) {
  return parse(pattern.c_str(),mode);
}


/*******************************************************************************/



// Match subject string, returning number of matches found
bool Regexp::match
(
  const char* string, int len,
  int* beg, int* end,
  int mode, int npar,
  int fm, int to
) const {
  
  if (!string || len<0 || npar<1 || NSUBEXP<npar) {
    fprintf(stderr,"Regexp::match: bad argument.\n");
  }
  
  if (fm<0) fm=0;
  if (to>len) to=len;
  if (fm<=to) {
    int abeg[NSUBEXP];
    int aend[NSUBEXP];
    Execute ms;
    if(!beg) beg=abeg;
    if(!end) end=aend;
    ms.str_beg=string;
    ms.str_end=string+len;
    ms.sub_beg=beg;
    ms.sub_end=end;
    ms.code=code;
    ms.npar=npar;
    ms.mode=mode;
    return ms.execute(string+fm,string+to);
  }
  return FALSE;
}


// Search for match in string
bool Regexp::match
(
  const std::string& string,
  int* beg, int* end,
  int mode, int npar,
  int fm, int to
) const {
  return match(string.c_str(),string.length(),beg,end,mode,npar,fm,to);
}

bool Regexp::match(const std::string &str, MatchData &mdata) const {
  return match(str.c_str(), str.length(),
               mdata.mBeg, mdata.mEnd, mdata.mMode, mdata.mDim,
               mdata.mFrom, mdata.mTo);
}


// Return substitution string
std::string Regexp::substitute
(
  const char* string, int len,
  int* beg, int* end,
  const std::string& replace, int npar
) {
  register int ch,n,i=0;
  std::string result;
  if (!string || len<0 || !beg || !end || npar<1 || NSUBEXP<npar) {
    fprintf(stderr,"Regexp::substitute: bad argument.\n");
  }
  while ((ch=replace[i++])!='\0') {
    if (ch=='&') {
      if (0<=beg[0] && end[0]<=len) {
        result.append(&string[beg[0]],end[0]-beg[0]);
      }
    } else if (ch=='\\' && '0'<=replace[i] && replace[i]<='9') {
      n=replace[i++]-'0';
      if (n<npar && 0<=beg[n] && end[n]<=len) {
        result.append(&string[beg[n]],end[n]-beg[n]);
      }
    } else {
      if (ch=='\\' && (replace[i]=='\\' || replace[i]=='&')) {
        ch=replace[i++];
      }
      //result.append(ch);
      char c = (char)ch;
      result.append(&c);
    }
  }
  return result;
}


// Return substitution string
std::string Regexp::substitute
(
  const std::string& string,
  int* beg, int* end,
  const std::string& replace, int npar
) {
  return substitute(string.c_str(),string.length(),beg,end,replace,npar);
}


// Equality
bool operator==(const Regexp &r1,const Regexp &r2) {
  return (
    (r1.code == r2.code) ||
    ((r1.code[0] == r2.code[0]) &&
     (memcmp(r1.code,r2.code,sizeof(int)*r1.code[0]) == 0))
  );
}


// Inequality
bool operator!=(const Regexp &r1,const Regexp &r2) {
  return !(r1 == r2);
}

// Clean up
Regexp::~Regexp() {
  if (code != fallback) {
    free(code);
    code = 0;
  }
}

}
