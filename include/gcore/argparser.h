/*

Copyright (C) 2009, 2010  Gaetan Guidet

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

#ifndef __gcore_argparser_h_
#define __gcore_argparser_h_

#include <gcore/string.h>
#include <gcore/list.h>
#include <gcore/status.h>

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
  
  class GCORE_API ArgParser {
    public:

      ArgParser(const FlagDesc *flags, int n);
      ~ArgParser();

      size_t argumentCount() const;
      bool getArgument(size_t idx, String &out) const;
      bool getArgument(size_t idx, float &out) const;
      bool getArgument(size_t idx, double &out) const;
      bool getArgument(size_t idx, int &out) const;
      bool getArgument(size_t idx, unsigned int &out) const;
      bool getArgument(size_t idx, bool &out) const;

      bool isFlagSet(const String &name) const;
      size_t flagOccurenceCount(const String &name) const;
      size_t flagArgumentCount(const String &name, size_t occurence=0) const;
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

      Status parse(int argc, char **argv);

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
