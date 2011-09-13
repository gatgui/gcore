/*

Copyright (C) 2011  Gaetan Guidet

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

#ifndef __gcore_bcfile_h_
#define __gcore_bcfile_h_

#include <gcore/config.h>

namespace gcore
{
   GCORE_API void WriteInt32(std::ostream &os, long val);
   GCORE_API void WriteUint32(std::ostream &os, unsigned long val);
   GCORE_API void WriteInt16(std::ostream &os, short val);
   GCORE_API void WriteUint16(std::ostream &os, unsigned short val);
   GCORE_API void WriteFloat(std::ostream &os, float val);
   GCORE_API void WriteDouble(std::ostream &os, double val);
   GCORE_API void WriteString(std::ostream &os, const char *str);
   GCORE_API void WriteString(std::ostream &os, const std::string &str);

   GCORE_API bool ReadInt32(std::istream &is, long &val);
   GCORE_API bool ReadUint32(std::istream &is, unsigned long &val);
   GCORE_API bool ReadInt16(std::istream &is, short &val);
   GCORE_API bool ReadUint16(std::istream &is, unsigned short &val);
   GCORE_API bool ReadFloat(std::istream &is, float &val);
   GCORE_API bool ReadDouble(std::istream &is, double &val);
   GCORE_API bool ReadString(std::istream &is, char **str);
   GCORE_API bool ReadString(std::istream &is, std::string &str);
   
   class GCORE_API BCFileElement
   {
   public:
      
      virtual ~BCFileElement() {}
      
      virtual size_t getByteSize() const = 0;
      
      virtual void writeHeader(std::ostream &os) const = 0;
      virtual void write(std::ostream &os) const = 0;
      
      virtual bool readHeader(std::istream &is) = 0;
      virtual bool read(std::istream &is) = 0;
   };
   
   class GCORE_API BCFile
   {
   protected:
      
      // At reading time, only the element offsets are read
      class ElementPlaceHolder : public BCFileElement
      {
      public:
         
         ElementPlaceHolder(unsigned long off);
         virtual ~ElementPlaceHolder();

         virtual size_t getByteSize() const;
         virtual void writeHeader(std::ostream &os) const;
         virtual void write(std::ostream &os) const;
         virtual bool readHeader(std::istream &is);
         virtual bool read(std::istream &is);

         unsigned long offset() const;
         
      protected:
         
         unsigned long mOffset;
      };
   
   public:
    
      BCFile();
      ~BCFile();

      void clearElements();
      bool addElement(const std::string &name, BCFileElement *e);
      bool hasElement(const std::string &name) const;
      bool readElement(const std::string &name, BCFileElement *elt);

      bool write(const std::string &filepath) const;
      // no read function, as we need the FileElement handlers to read properly
      // just bookkeep the offset if the different elements
      // -> that's the reason an internal file object is kept
      bool readTOC(const std::string &filepath);

      void doneReading();

   protected:

      // write file version 0.1
      void write_0_1(std::ofstream &ofile, size_t baseOff) const;

      // read file version 0.1 TOC
      bool read_0_1(std::ifstream &ifile);

   protected:

      std::map<std::string, BCFileElement*> mElements;
      std::ifstream mInFile;
      std::vector<ElementPlaceHolder*> mPlaceHolders;
   };
   
}

#endif


