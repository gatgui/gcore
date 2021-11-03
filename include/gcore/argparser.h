/*
MIT License

Copyright (c) 2009 Gaetan Guidet

This file is part of gcore.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __gcore_argparser_h_
#define __gcore_argparser_h_

#include <gcore/string.h>
#include <gcore/list.h>

#define ACCEPTS_NOFLAG_ARGUMENTS(arity) {(gcore::FlagDesc::Option)0, "", "", arity}

namespace gcore {

  struct GCORE_API FlagDesc {
    enum Option {
      FT_OPTIONAL = 0x01,  // flag is optional
      FT_NEEDED = 0x02,    // flag is required
      FT_MULTI = 0x04      // flag can appear several times
    };
    Option opts;
    String longname;  // -<shortName>
    String shortname; // --<longName>
    int arity;             // <0: any
  };
  
  class GCORE_API ArgParserError : public std::exception {
    public:
      ArgParserError(const String &message);
      virtual ~ArgParserError() throw();
      virtual const char* what() const throw();
    protected:
      String mMessage;
  };

  class GCORE_API ArgParser {
    public:

      ArgParser(const FlagDesc *flags, int n);
      ~ArgParser();

      size_t getArgumentCount() const;
      bool getArgument(size_t idx, String &out) const;
      bool getArgument(size_t idx, float &out) const;
      bool getArgument(size_t idx, double &out) const;
      bool getArgument(size_t idx, int &out) const;
      bool getArgument(size_t idx, unsigned int &out) const;
      bool getArgument(size_t idx, bool &out) const;

      bool isFlagSet(const String &name) const;
      size_t getFlagOccurenceCount(const String &name) const;// if flag is multi
      size_t getFlagArgumentCount(const String &name, size_t occurence=0) const;
      bool getFlagArgument(const String &name, size_t idx, String &out) const;
      bool getFlagArgument(const String &name, size_t idx, float &out) const;
      bool getFlagArgument(const String &name, size_t idx, double &out) const;
      bool getFlagArgument(const String &name, size_t idx, int &out) const;
      bool getFlagArgument(const String &name, size_t idx, unsigned int &out) const;
      bool getFlagArgument(const String &name, size_t idx, bool &out) const;
      bool getMultiFlagArgument(const String &name, size_t occurence, size_t idx, String &out) const;
      bool getMultiFlagArgument(const String &name, size_t occurence,  size_t idx, float &out) const;
      bool getMultiFlagArgument(const String &name, size_t occurence,  size_t idx, double &out) const;
      bool getMultiFlagArgument(const String &name, size_t occurence,  size_t idx, int &out) const;
      bool getMultiFlagArgument(const String &name, size_t occurence,  size_t idx, unsigned int &out) const;
      bool getMultiFlagArgument(const String &name, size_t occurence,  size_t idx, bool &out) const;

      void parse(int argc, char **argv) throw (ArgParserError);

    protected:

      FlagDesc* findLongFlag(const String &name);
      FlagDesc* findShortFlag(const String &name);
      void reset();
      
    protected:
      
      typedef StringList FlagValues;
      typedef List<FlagValues> FlagOccurencesValues;
      typedef std::map<String, int> FlagsMap;

      bool mNoFlagOn;
      int mNoFlagCount;
      List<FlagDesc> mFlags;
      StringList mArgs;
      FlagsMap mFlagsMap;
      List<FlagOccurencesValues> mDatas;

    private:

      ArgParser(){}
      ArgParser& operator=(const ArgParser&) {return *this;}
  };

}

#endif
