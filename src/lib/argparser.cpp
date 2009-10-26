/*

Copyright (C) 2009  Gaetan Guidet

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

#include <gcore/argparser.h>

#ifdef _WIN32
# if _MSC_VER >= 1400
    // Visual Studio 2005 or above
#		define strcasecmp _stricmp
# else
# 	define strcasecmp stricmp
# endif
#endif

using namespace std;

namespace gcore {


ArgParserError::ArgParserError(const std::string &message)
  :mMessage(message) {
}

ArgParserError::~ArgParserError() throw() {
}

const char* ArgParserError::what() const throw() {
  return mMessage.c_str();
}


ArgParser::ArgParser(const FlagDesc *flags, int n)
  : mNoFlagOn(false) {
  // Better flag check
  for (int i=0; i<n; ++i) {
    bool exists = false;
    for (size_t j=0; j<mFlags.size(); ++j) {
      if (mFlags[j].longname == flags[i].longname) {
        exists = true;
        break;
      }
    }
    if (!exists) {
      if (flags[i].longname.length() == 0 &&
          flags[i].shortname.length() == 0) {
        if (flags[i].arity != 0) {
          mNoFlagOn = true;
          mNoFlagCount = flags[i].arity;
        } else {
          mNoFlagOn = false;
          mNoFlagCount = 0;
        }
      } else {
        mFlags.push_back(flags[i]);
      }
    }
  }
}

ArgParser::~ArgParser() {
}

size_t ArgParser::getArgumentCount() const {
  return mArgs.size();
}

bool ArgParser::getArgument(size_t idx, string &out) const {
  if (idx < mArgs.size()) {
    out = mArgs[idx];
    return true;
  }
  return false;
}

bool ArgParser::getArgument(size_t idx, float &out) const {
  if (idx < mArgs.size()) {
    return (sscanf(mArgs[idx].c_str(), "%f", &out) == 1);
  }
  return false;
}

bool ArgParser::getArgument(size_t idx, double &out) const {
  if (idx < mArgs.size()) {
    return (sscanf(mArgs[idx].c_str(), "%lf", &out) == 1);
  }
  return false;
}

bool ArgParser::getArgument(size_t idx, int &out) const {
  if (idx < mArgs.size()) {
    return (sscanf(mArgs[idx].c_str(), "%d", &out) == 1);
  }
  return false;
}

bool ArgParser::getArgument(size_t idx, unsigned int &out) const {
  if (idx < mArgs.size()) {
    return (sscanf(mArgs[idx].c_str(), "%u", &out) == 1);
  }
  return false;
}

bool ArgParser::getArgument(size_t idx, bool &out) const {
  if (idx < mArgs.size()) {
    if ((mArgs[idx] == "0") ||
        !strcasecmp(mArgs[idx].c_str(), "false")) {
      out = false;
      return true;
    } else if ((mArgs[idx] == "1") ||
              !strcasecmp(mArgs[idx].c_str(), "true")) {
      out = true;
      return true;
    }
  }
  return false;
}

size_t ArgParser::getFlagOccurenceCount(const string &name) const {
  FlagsMap::const_iterator it = mFlagsMap.find(name);
  if (it != mFlagsMap.end()) {
    return (mDatas[it->second].size());
  }
  return 0;
}

size_t ArgParser::getFlagArgumentCount(const string &name, size_t occurence) const {
  FlagsMap::const_iterator it = mFlagsMap.find(name);
  if (it != mFlagsMap.end()) {
    const FlagOccurencesValues &data = mDatas[it->second];
    if (occurence < data.size()) {
      return data[occurence].size();
    }
  }
  return 0;
}

bool ArgParser::getMultiFlagArgument(const string &name, size_t occurence, size_t idx, string &out) const {
  FlagsMap::const_iterator it = mFlagsMap.find(name);
  if (it != mFlagsMap.end()) {
    const FlagOccurencesValues &data = mDatas[it->second];
    if (occurence < data.size()) {
      if (idx < data[occurence].size()) {
        out = data[occurence][idx];
        return true;
      }
    }
  }
  return false;
}

bool ArgParser::getMultiFlagArgument(const string &name, size_t occurence,  size_t idx, float &out) const {
  FlagsMap::const_iterator it = mFlagsMap.find(name);
  if (it != mFlagsMap.end()) {
    const FlagOccurencesValues &data = mDatas[it->second];
    if (occurence < data.size()) {
      if (idx < data[occurence].size()) {
        return (sscanf(data[occurence][idx].c_str(), "%f", &out) == 1);
      }
    }
  }
  return false;
}

bool ArgParser::getMultiFlagArgument(const string &name, size_t occurence,  size_t idx, double &out) const {
  FlagsMap::const_iterator it = mFlagsMap.find(name);
  if (it != mFlagsMap.end()) {
    const FlagOccurencesValues &data = mDatas[it->second];
    if (occurence < data.size()) {
      if (idx < data[occurence].size()) {
        return (sscanf(data[occurence][idx].c_str(), "%lf", &out) == 1);
      }
    }
  }
  return false;
}

bool ArgParser::getMultiFlagArgument(const string &name, size_t occurence,  size_t idx, int &out) const {
  FlagsMap::const_iterator it = mFlagsMap.find(name);
  if (it != mFlagsMap.end()) {
    const FlagOccurencesValues &data = mDatas[it->second];
    if (occurence < data.size()) {
      if (idx < data[occurence].size()) {
        return (sscanf(data[occurence][idx].c_str(), "%d", &out) == 1);
      }
    }
  }
  return false;
}

bool ArgParser::getMultiFlagArgument(const string &name, size_t occurence,  size_t idx, unsigned int &out) const {
  FlagsMap::const_iterator it = mFlagsMap.find(name);
  if (it != mFlagsMap.end()) {
    const FlagOccurencesValues &data = mDatas[it->second];
    if (occurence < data.size()) {
      if (idx < data[occurence].size()) {
        return (sscanf(data[occurence][idx].c_str(), "%u", &out) == 1);
      }
    }
  }
  return false;
}

bool ArgParser::getMultiFlagArgument(const string &name, size_t occurence,  size_t idx, bool &out) const {
  FlagsMap::const_iterator it = mFlagsMap.find(name);
  if (it != mFlagsMap.end()) {
    const FlagOccurencesValues &data = mDatas[it->second];
    if (occurence < data.size()) {
      if (idx < data[occurence].size()) {
        if ((data[occurence][idx] == "0") ||
            !strcasecmp(data[occurence][idx].c_str(), "false")) {
          out = false;
          return true;
        } else if ((data[occurence][idx] == "1") ||
                  !strcasecmp(data[occurence][idx].c_str(), "true")) {
          out = true;
          return true;
        }
      }
    }
  }
  return false;
}

bool ArgParser::getFlagArgument(const string &name, size_t idx, string &out) const {
  return getMultiFlagArgument(name, 0, idx, out);
}

bool ArgParser::getFlagArgument(const string &name, size_t idx, float &out) const {
  return getMultiFlagArgument(name, 0, idx, out);
}

bool ArgParser::getFlagArgument(const string &name, size_t idx, double &out) const {
  return getMultiFlagArgument(name, 0, idx, out);
}

bool ArgParser::getFlagArgument(const string &name, size_t idx, int &out) const {
  return getMultiFlagArgument(name, 0, idx, out);
}

bool ArgParser::getFlagArgument(const string &name, size_t idx, unsigned int &out) const {
  return getMultiFlagArgument(name, 0, idx, out);
}

bool ArgParser::getFlagArgument(const string &name, size_t idx, bool &out) const {
  return getMultiFlagArgument(name, 0, idx, out);
}

bool ArgParser::isFlagSet(const string &name) const {
  return (mFlagsMap.find(name) != mFlagsMap.end());
}

void ArgParser::parse(int argc, char **argv) throw (ArgParserError) {

  reset();

  if (argc < 0 || !argv) {
    throw ArgParserError("Invalid arguments");
  }

  int carg = 0;
  int valcount = 0;
  FlagDesc *cflag = 0;
  int cdata = -1;
  int cvalues = -1;
  char flag[64];

  while (carg < argc) {
    
    FlagDesc *nflag = 0;
    bool isFlag = false;
    
    if (sscanf(argv[carg], "--%s[A-Za-z0-9_-]", flag) == 1) {
      isFlag = true;
      nflag = findLongFlag(flag);
    } else if (sscanf(argv[carg], "-%s[A-Za-z0-9_-]", flag) == 1) {
      isFlag = true;
      nflag = findShortFlag(flag);
    }

    if (isFlag) {
      
      if (!nflag) {
        ostringstream oss;
        oss << "Unknown flag: \"" << flag << "\"";
        throw ArgParserError(oss.str());
      }

      if (cflag) {
        if (cflag->arity>0 && valcount!=cflag->arity) {
          ostringstream oss;
          oss << "\"" << cflag->longname << "\" flag requires " << cflag->arity << " value(s)";
          throw ArgParserError(oss.str());
        } else if (cflag->arity==0 && valcount>0) {
          ostringstream oss;
          oss << "\"" << cflag->longname << "\" flag does not accept any value";
          throw ArgParserError(oss.str());
        }
      }
      
      cflag = nflag;
      valcount = 0;

      FlagsMap::const_iterator it = mFlagsMap.find(cflag->longname);
      if (it == mFlagsMap.end()) {
        it = mFlagsMap.find(cflag->shortname);
      }
      if (it == mFlagsMap.end()) {
        cdata = (int)mDatas.size();
        mFlagsMap[cflag->shortname] = cdata;
        mFlagsMap[cflag->longname] = cdata;
        mDatas.push_back(FlagOccurencesValues());
      } else {
        if (!(cflag->opts & FlagDesc::FT_MULTI)) {
          ostringstream oss;
          oss << "\"" << cflag->longname << "\" flag is single usage";
          throw ArgParserError(oss.str());
        }
        cdata = it->second;
      }
      if (cflag->arity != 0) {
        cvalues = (int)mDatas[cdata].size();
        mDatas[cdata].push_back(FlagValues());
      } else {
        cvalues = -1;
      }
    } else {
      if (cvalues != -1 && valcount != cflag->arity) {
        valcount++;
        mDatas[cdata][cvalues].push_back(string(argv[carg]));
      } else {
        cflag = 0;
        cdata = -1;
        cvalues = -1;
        valcount = 0;
        if (mNoFlagOn) {
          if (mNoFlagCount == -1 ||
              (int)(mArgs.size()) < mNoFlagCount) {
            mArgs.push_back(string(argv[carg]));
          } else {
            ostringstream oss;
            oss << "Only " << mNoFlagCount << " non-flag value(s) accepted";
            throw ArgParserError(oss.str());
          }
        } else {
          ostringstream oss;
          oss << "Only flags arguments are accepted (Arg " << carg << ": " << argv[carg] << ")";
          throw ArgParserError(oss.str());
        }
      }
    }
    ++carg;
  }
  if (cflag) {
    if (cflag->arity>0 && cflag->arity!=valcount) {
      ostringstream oss;
      oss << "\"" << cflag->longname << "\" flag requires " << cflag->arity << " value(s)";
      throw ArgParserError(oss.str());
    }
  }
  if (mNoFlagOn && mNoFlagCount>0) {
    if ((int)mArgs.size() != mNoFlagCount) {
      ostringstream oss;
      oss << mNoFlagCount << " non-flag value(s) required";
      throw ArgParserError(oss.str());
    }
  }
  if (carg == argc) {
    // check if all required flags are set !
    for (size_t i=0; i<mFlags.size(); ++i) {
      if (mFlags[i].opts & FlagDesc::FT_NEEDED) {
        if (mFlagsMap.find(mFlags[i].longname) == mFlagsMap.end()) {
          ostringstream oss;
          oss << "\"" << mFlags[i].longname << "\" flag is required";
          throw ArgParserError(oss.str());
        }
      }
    }
  }
}

FlagDesc* ArgParser::findLongFlag(const string &name) {
  size_t i;
  for (i=0; i<mFlags.size(); ++i) {
    if (mFlags[i].longname == name) {
      return &(mFlags[i]);
    }
  }
  return 0;
}

FlagDesc* ArgParser::findShortFlag(const string &name) {
  size_t i;
  for (i=0; i<mFlags.size(); ++i) {
    if (mFlags[i].shortname == name) {
      return &(mFlags[i]);
    }
  }
  return 0;
}

void ArgParser::reset() {
  mFlagsMap.clear();
  mDatas.clear();
  mArgs.clear();
}

}

