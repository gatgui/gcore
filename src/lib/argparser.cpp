/*

Copyright (C) 2009~  Gaetan Guidet

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
#include <gcore/unicode.h>

namespace gcore
{

ArgParser::ArgParser(const FlagDesc *flags, int n)
   : mNoFlagOn(false)
   , mNoFlagCount(0)
{
   // Better flag check
   for (int i=0; i<n; ++i)
   {
      bool exists = false;
      
      for (size_t j=0; j<mFlags.size(); ++j)
      {
         if (mFlags[j].longname == flags[i].longname)
         {
            exists = true;
            break;
         }
      }
      
      if (!exists)
      {
         if (flags[i].longname.length() == 0 &&
             flags[i].shortname.length() == 0)
         {
            if (flags[i].arity != 0)
            {
               mNoFlagOn = true;
               mNoFlagCount = flags[i].arity;
            }
            else
            {
               mNoFlagOn = false;
               mNoFlagCount = 0;
            }
         }
         else
         {
            mFlags.push(flags[i]);
         }
      }
   }
}

ArgParser::~ArgParser()
{
}

size_t ArgParser::argumentCount() const
{
   return mArgs.size();
}

bool ArgParser::getArgument(size_t idx, String &out) const
{
   if (idx < mArgs.size())
   {
      out = mArgs[idx];
      return true;
   }
   return false;
}

bool ArgParser::getArgument(size_t idx, float &out) const
{
   return mArgs[idx].toFloat(out);
}

bool ArgParser::getArgument(size_t idx, double &out) const
{
   return mArgs[idx].toDouble(out);
}

bool ArgParser::getArgument(size_t idx, int &out) const
{
   return mArgs[idx].toInt(out);
}

bool ArgParser::getArgument(size_t idx, unsigned int &out) const
{
   return mArgs[idx].toUInt(out);
}

bool ArgParser::getArgument(size_t idx, bool &out) const {
   return mArgs[idx].toBool(out);
}

size_t ArgParser::flagOccurenceCount(const String &name) const
{
   FlagsMap::const_iterator it = mFlagsMap.find(name);
   if (it != mFlagsMap.end())
   {
      return (mDatas[it->second].size());
   }
   return 0;
}

size_t ArgParser::flagArgumentCount(const String &name, size_t occurence) const
{
   FlagsMap::const_iterator it = mFlagsMap.find(name);
   if (it != mFlagsMap.end())
   {
      const FlagOccurencesValues &data = mDatas[it->second];
      if (occurence < data.size())
      {
         return data[occurence].size();
      }
   }
   return 0;
}

bool ArgParser::getMultiFlagArgument(const String &name, size_t occurence, size_t idx, String &out) const
{
   FlagsMap::const_iterator it = mFlagsMap.find(name);
   if (it != mFlagsMap.end())
   {
      const FlagOccurencesValues &data = mDatas[it->second];
      if (occurence < data.size())
      {
         if (idx < data[occurence].size())
         {
            out = data[occurence][idx];
            return true;
         }
      }
   }
   return false;
}

bool ArgParser::getMultiFlagArgument(const String &name, size_t occurence,  size_t idx, float &out) const
{
   FlagsMap::const_iterator it = mFlagsMap.find(name);
   if (it != mFlagsMap.end())
   {
      const FlagOccurencesValues &data = mDatas[it->second];
      if (occurence < data.size())
      {
         if (idx < data[occurence].size())
         {
            return data[occurence][idx].toFloat(out);
         }
      }
   }
   return false;
}

bool ArgParser::getMultiFlagArgument(const String &name, size_t occurence,  size_t idx, double &out) const
{
   FlagsMap::const_iterator it = mFlagsMap.find(name);
   if (it != mFlagsMap.end())
   {
      const FlagOccurencesValues &data = mDatas[it->second];
      if (occurence < data.size())
      {
         if (idx < data[occurence].size())
         {
            return data[occurence][idx].toDouble(out);
         }
      }
   }
   return false;
}

bool ArgParser::getMultiFlagArgument(const String &name, size_t occurence,  size_t idx, int &out) const
{
   FlagsMap::const_iterator it = mFlagsMap.find(name);
   if (it != mFlagsMap.end())
   {
      const FlagOccurencesValues &data = mDatas[it->second];
      if (occurence < data.size())
      {
         if (idx < data[occurence].size())
         {
            return data[occurence][idx].toInt(out);
         }
      }
   }
   return false;
}

bool ArgParser::getMultiFlagArgument(const String &name, size_t occurence,  size_t idx, unsigned int &out) const
{
   FlagsMap::const_iterator it = mFlagsMap.find(name);
   if (it != mFlagsMap.end())
   {
      const FlagOccurencesValues &data = mDatas[it->second];
      if (occurence < data.size())
      {
         if (idx < data[occurence].size())
         {
            return data[occurence][idx].toUInt(out);
         }
      }
   }
   return false;
}

bool ArgParser::getMultiFlagArgument(const String &name, size_t occurence,  size_t idx, bool &out) const
{
   FlagsMap::const_iterator it = mFlagsMap.find(name);
   if (it != mFlagsMap.end())
   {
      const FlagOccurencesValues &data = mDatas[it->second];
      if (occurence < data.size())
      {
         if (idx < data[occurence].size())
         {
            return data[occurence][idx].toBool(out);
         }
      }
   }
   return false;
}

bool ArgParser::getFlagArgument(const String &name, size_t idx, String &out) const
{
   return getMultiFlagArgument(name, 0, idx, out);
}

bool ArgParser::getFlagArgument(const String &name, size_t idx, float &out) const
{
   return getMultiFlagArgument(name, 0, idx, out);
}

bool ArgParser::getFlagArgument(const String &name, size_t idx, double &out) const
{
   return getMultiFlagArgument(name, 0, idx, out);
}

bool ArgParser::getFlagArgument(const String &name, size_t idx, int &out) const
{
   return getMultiFlagArgument(name, 0, idx, out);
}

bool ArgParser::getFlagArgument(const String &name, size_t idx, unsigned int &out) const
{
   return getMultiFlagArgument(name, 0, idx, out);
}

bool ArgParser::getFlagArgument(const String &name, size_t idx, bool &out) const
{
   return getMultiFlagArgument(name, 0, idx, out);
}

bool ArgParser::isFlagSet(const String &name) const
{
   return (mFlagsMap.find(name) != mFlagsMap.end());
}

static bool GetUTF8String(const char *s, String &out)
{
   if (!s)
   {
      out = "";
      return true;
   }
   
   if (!IsUTF8(s))
   {
      return LocaleToUTF8(s, out);
   }
   else
   {
      out = s;
      return true;
   }
}

Status ArgParser::parse(int argc, char **argv)
{
   reset();

   if (argc < 0 || !argv)
   {
      return Status(false, "Invalid arguments.");
   }

   int carg = 0;
   int valcount = 0;
   FlagDesc *cflag = 0;
   int cdata = -1;
   int cvalues = -1;
   char flag[64];
   String arg;

   while (carg < argc)
   {
      FlagDesc *nflag = 0;
      bool isFlag = false;
      
      if (sscanf(argv[carg], "--%s[A-Za-z0-9_-]", flag) == 1)
      {
         isFlag = true;
         nflag = findLongFlag(flag);
      }
      else if (sscanf(argv[carg], "-%s[A-Za-z0-9_-]", flag) == 1)
      {
         isFlag = true;
         nflag = findShortFlag(flag);
      }

      if (isFlag)
      {
         if (!nflag)
         {
            return Status(false, "Unknown flag: \"%s\"", flag);
         }

         if (cflag)
         {
            if (cflag->arity>0 && valcount!=cflag->arity)
            {
               return Status(false, "\"%s\" flag requires %d value(s)", cflag->longname.c_str(), cflag->arity);
            }
            else if (cflag->arity==0 && valcount>0)
            {
               return Status(false, "\"%s\" flag does not accept any value", cflag->longname.c_str());
            }
         }
         
         cflag = nflag;
         valcount = 0;

         FlagsMap::const_iterator it = mFlagsMap.find(cflag->longname);
         if (it == mFlagsMap.end())
         {
            it = mFlagsMap.find(cflag->shortname);
         }
         
         if (it == mFlagsMap.end())
         {
            cdata = (int)mDatas.size();
            mFlagsMap[cflag->shortname] = cdata;
            mFlagsMap[cflag->longname] = cdata;
            mDatas.push(FlagOccurencesValues());
         
         }
         else
         {
            if (!(cflag->opts & FlagDesc::FT_MULTI))
            {
               return Status(false, "\"%s\" flag is single usage", cflag->longname.c_str());
            }
            cdata = it->second;
         }
         
         if (cflag->arity != 0)
         {
            cvalues = (int)mDatas[cdata].size();
            mDatas[cdata].push(FlagValues());
         }
         else
         {
            cvalues = -1;
         }
      }
      else
      {
         if (cvalues != -1 && valcount != cflag->arity)
         {
            if (!GetUTF8String(argv[carg], arg))
            {
               return Status(false, "Unsupported command line argument encoding. (Arg %d: %s)", carg, argv[carg]);
            }
            else
            {
               valcount++;
               mDatas[cdata][cvalues].push(arg);
            }
            
         }
         else
         {
            cflag = 0;
            cdata = -1;
            cvalues = -1;
            valcount = 0;
            if (mNoFlagOn)
            {
               if (mNoFlagCount == -1 || (int)(mArgs.size()) < mNoFlagCount)
               {
                  if (!GetUTF8String(argv[carg], arg))
                  {
                     return Status(false, "Unsupported command line argument encoding. (Arg %d: %s)", carg, argv[carg]);
                  }
                  else
                  {
                     mArgs.push(arg);
                  }
               }
               else
               {
                  return Status(false, "Only %d non-flag value(s) accepted", mNoFlagCount);
               }
            }
            else
            {
               return Status(false, "Only flags arguments are accepted (Arg %d: %s)", carg, argv[carg]);
            }
         }
      }
      ++carg;
   }
   
   if (cflag)
   {
      if (cflag->arity > 0 && cflag->arity != valcount)
      {
         return Status(false, "\"%s\" flag requires %d value(s)", cflag->longname.c_str(), cflag->arity);
      }
   }
   
   if (mNoFlagOn && mNoFlagCount>0)
   {
      if ((int)mArgs.size() != mNoFlagCount)
      {
         return Status(false, "%d non-flag value(s) required", mNoFlagCount);
      }
   }
   
   if (carg == argc)
   {
      // check if all required flags are set !
      for (size_t i=0; i<mFlags.size(); ++i)
      {
         if (mFlags[i].opts & FlagDesc::FT_NEEDED)
         {
            if (mFlagsMap.find(mFlags[i].longname) == mFlagsMap.end())
            {
               return Status(false, "\"%s\" flag is required", mFlags[i].longname.c_str());
            }
         }
      }
   }
   
   return Status(true);
}

FlagDesc* ArgParser::findLongFlag(const String &name)
{
   size_t i;
   for (i=0; i<mFlags.size(); ++i)
   {
      if (mFlags[i].longname == name)
      {
         return &(mFlags[i]);
      }
   }
   return 0;
}

FlagDesc* ArgParser::findShortFlag(const String &name)
{
   size_t i;
   for (i=0; i<mFlags.size(); ++i)
   {
      if (mFlags[i].shortname == name)
      {
         return &(mFlags[i]);
      }
   }
   return 0;
}

void ArgParser::reset()
{
   mFlagsMap.clear();
   mDatas.clear();
   mArgs.clear();
}

} // gcore

