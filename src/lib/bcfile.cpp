#include <gcore/bcfile.h>
#include <gcore/log.h>

namespace gcore
{

void WriteInt32(std::ostream &os, long val)
{
   unsigned char buf[4];
   buf[0] = (unsigned char)(val & 0x000000FF);
   buf[1] = (unsigned char)((val & 0x0000FF00) >> 8);
   buf[2] = (unsigned char)((val & 0x00FF0000) >> 16);
   buf[3] = (unsigned char)((val & 0xFF000000) >> 24);
   os.write((char*)buf, 4);
}

void WriteUint32(std::ostream &os, unsigned long val)
{
   unsigned char buf[4];
   buf[0] = (unsigned char)(val & 0x000000FF);
   buf[1] = (unsigned char)((val & 0x0000FF00) >> 8);
   buf[2] = (unsigned char)((val & 0x00FF0000) >> 16);
   buf[3] = (unsigned char)((val & 0xFF000000) >> 24);
   os.write((char*)buf, 4);
}

void WriteInt16(std::ostream &os, short val)
{
   unsigned char buf[2];
   buf[0] = (unsigned char)(val & 0x000000FF);
   buf[1] = (unsigned char)((val & 0x0000FF00) >> 8);
   os.write((char*)buf, 2);
}

void WriteUint16(std::ostream &os, unsigned short val)
{
   unsigned char buf[2];
   buf[0] = (unsigned char)(val & 0x000000FF);
   buf[1] = (unsigned char)((val & 0x0000FF00) >> 8);
   os.write((char*)buf, 2);
}

void WriteFloat(std::ostream &os, float val)
{
   os.write((char*)&val, sizeof(float));
}

void WriteDouble(std::ostream &os, double val)
{
   os.write((char*)&val, sizeof(double));
}

void WriteString(std::ostream &os, const char *str)
{
   char eos = '\0';
   size_t len = strlen(str);
   WriteUint32(os, (unsigned long)len+1);
   os.write(str, len);
   os.write(&eos, 1);
}

void WriteString(std::ostream &os, const std::string &str)
{
   char eos = '\0';
   WriteUint32(os, (unsigned long)str.length()+1);
   os.write(str.c_str(), str.length());
   os.write(&eos, 1);
}

bool ReadInt32(std::istream &is, long &val)
{
   unsigned char buf[4];
   is.read((char*)buf, 4);
   val = 0;
   val |= buf[3];
   val <<= 8;
   val |= buf[2];
   val <<= 8;
   val |= buf[1];
   val <<= 8;
   val |= buf[0];
   return (!is.bad());
}

bool ReadUint32(std::istream &is, unsigned long &val)
{
   unsigned char buf[4];
   is.read((char*)buf, 4);
   val = 0;
   val |= buf[3];
   val <<= 8;
   val |= buf[2];
   val <<= 8;
   val |= buf[1];
   val <<= 8;
   val |= buf[0];
   return (!is.bad());
}

bool ReadInt16(std::istream &is, short &val)
{
   unsigned char buf[2];
   is.read((char*)buf, 2);
   val = 0;
   val |= buf[1];
   val <<= 8;
   val |= buf[0];
   return (!is.bad());
}

bool ReadUint16(std::istream &is, unsigned short &val)
{
   unsigned char buf[2];
   is.read((char*)buf, 2);
   val = 0;
   val |= buf[1];
   val <<= 8;
   val |= buf[0];
   return (!is.bad());
}

bool ReadFloat(std::istream &is, float &val)
{
   is.read((char*)&val, sizeof(float));
   return (!is.bad());
}

bool ReadDouble(std::istream &is, double &val)
{
   is.read((char*)&val, sizeof(double));
   return (!is.bad());
}

bool ReadString(std::istream &is, char **str)
{
   // len includes the trailing eos
   unsigned long len;
   if (!ReadUint32(is, len) || is.eof())
   {
      return false;
   }
   *str = new char[len];
   is.read(*str, len);
   return (!is.bad());
}

bool ReadString(std::istream &is, std::string &str)
{
   // len includes the trailing eos
   char eos = '\0';
   unsigned long len;
   if (!ReadUint32(is, len) || is.eof())
   {
      return false;
   }
   std::getline(is, str, eos);
   if (str.length() != len-1)
   {
      return false;
   }
   return (!is.bad());
}

// ---

BCFile::ElementPlaceHolder::ElementPlaceHolder(unsigned long off, size_t sz)
   : mOffset(off), mSize(sz), mContent(0)
{
}

BCFile::ElementPlaceHolder::~ElementPlaceHolder()
{
   if (mContent)
   {
      delete[] mContent;
      mContent = 0;
   }
}

size_t BCFile::ElementPlaceHolder::getByteSize() const
{
   return mSize;
}

void BCFile::ElementPlaceHolder::writeHeader(std::ostream &) const
{
}

void BCFile::ElementPlaceHolder::write(std::ostream &os) const
{
   if (mSize > 0 && mContent != 0)
   {
      os.write(mContent, mSize);
   }
}

bool BCFile::ElementPlaceHolder::readHeader(std::istream &)
{
   return true;
}

bool BCFile::ElementPlaceHolder::read(std::istream &is)
{
   if (mSize > 0)
   {
      if (mContent == 0)
      {
         mContent = new char[mSize];
         is.read(mContent, mSize);
         return (is.good());
      }
      else
      {
         return true;
      }
   }
   else
   {
      return false;
   }
}

unsigned long BCFile::ElementPlaceHolder::offset() const
{
   return mOffset;
}

// ---

BCFile::BCFile()
{
}

BCFile::~BCFile()
{
   clearElements();
   mInFile.close();
}

void BCFile::clearElements()
{
   // BCFile do NOT own the BCFileElement pointers (except for
   // the ones it created -> mPlaceHolders)
   mElements.clear();
   for (size_t i=0; i<mPlaceHolders.size(); ++i)
   {
     delete mPlaceHolders[i];
   }
   mPlaceHolders.clear();
}

bool BCFile::addElement(const std::string &name, BCFileElement *e)
{
   if (mElements.find(name) == mElements.end())
   {
      mElements[name] = e;
      return true;
   }
   else
   {
      return false;
   }
}

bool BCFile::hasElement(const std::string &name) const
{
   return (mElements.find(name) != mElements.end());
}

bool BCFile::write(const std::string &filepath, bool preserveData) const
{
   // first read all placeholders data if mInFile is open
   if (preserveData && mInFile.is_open())
   {
      std::map<std::string, BCFileElement*>::iterator elt = mElements.begin();
      while (elt != mElements.end())
      {
         ElementPlaceHolder *feph = dynamic_cast<ElementPlaceHolder*>(elt->second);
         if (feph)
         {
            mInFile.seekg(feph->offset(), std::ios::beg);
            if (!mInFile.good() || !feph->read(mInFile))
            {
               // remove element
               std::map<std::string, BCFileElement*>::iterator tmp = elt;
               ++elt;
               mElements.erase(tmp);
               
               // delete placeholder
               std::vector<ElementPlaceHolder*>::iterator ph = std::find(mPlaceHolders.begin(), mPlaceHolders.end(), feph);
               if (ph != mPlaceHolders.end())
               {
                  mPlaceHolders.erase(ph);
               }
               delete feph;
               
               // clear file error and continue looping for place holders
               mInFile.clear();
               
               continue;
            }
         }
         ++elt;
      }
   }
   
   std::ofstream ofile(filepath.c_str(), std::ofstream::binary);

   if (!ofile.is_open())
   {
      return false;
   }
   
   // BFC stands for "Binary File Container"
   ofile.write("BCFv", 4);
   // Major version
   WriteUint16(ofile, 0);
   // Minor version
   WriteUint16(ofile, 2);
   
   size_t baseOff = 8; // 4 char + 2 shorts
   
   write_0_2(ofile, baseOff);
   
   ofile.close();
   
   return true;
}

bool BCFile::readTOC(const std::string &filepath)
{
   char buffer[8];
   
   if (mInFile.is_open())
   {
      mInFile.close();
   }
   clearElements();
   
   mInFile.open(filepath.c_str(), std::ifstream::binary);
   
   if (!mInFile.good())
   {
      return false;
   }
   
   bool rv = false;
   
   mInFile.read(buffer, 4);
   if (!mInFile.good())
   {
      // use good here because we also want to test EOF
      mInFile.close();
      return false;
   }
   
   if (!strncmp(buffer, "BCFv", 4))
   {
      unsigned short majVer, minVer;

      if (!ReadUint16(mInFile, majVer) ||
          !ReadUint16(mInFile, minVer))
      {
         mInFile.close();
         return false;
      }

      if (majVer == 0)
      {
         if (minVer == 1)
         {
            rv = read_0_1(mInFile);
         }
         else if (minVer == 2)
         {
            rv = read_0_2(mInFile);
         }
      }
   }
   
   if (!rv)
   {
      doneReading();
   }
   
   return rv;
}

bool BCFile::readElement(const std::string &name, BCFileElement *elt)
{
   std::map<std::string, BCFileElement*>::iterator it = mElements.find(name);
   
   if (it == mElements.end())
   {
      return false;
   }
   
   ElementPlaceHolder *feph = dynamic_cast<ElementPlaceHolder*>(it->second);
   
   if (!feph)
   {
      // if it is not a place holder, the element has already been read
      // it is not a failure if pointer matches, but it won't re-read the element
      return (elt == it->second);
   }
   
   mInFile.seekg(feph->offset(), std::ios::beg);
   
   if (!mInFile.good())
   {
      return false;
   }
   
   if (!elt->readHeader(mInFile))
   {
      return false;
   }
   
   if (!elt->read(mInFile))
   {
      return false;
   }
   
   // delete place holder
   
   std::vector<ElementPlaceHolder*>::iterator eit = std::find(mPlaceHolders.begin(), mPlaceHolders.end(), feph);
   
   if (eit != mPlaceHolders.end())
   {
      mPlaceHolders.erase(eit);
   }
   
   delete feph;
   
   // replace it by the read element
   
   mElements[it->first] = elt;
   
   return true;
}

void BCFile::doneReading()
{
   mInFile.close();
   clearElements();
}

// write file version 0.1
void BCFile::write_0_1(std::ofstream &ofile, size_t baseOff) const
{
   baseOff += 4; // number of elements
   
   std::map<std::string, BCFileElement*>::const_iterator elt = mElements.begin();
   
   // optimize that by calculating index size as elements are added
   while (elt != mElements.end())
   {
      baseOff += 4; // element name length
      baseOff += elt->first.length() + 1; // element name (includes the \0)
      baseOff += 4; // element data offset in file
      ++elt;
   }
   
   WriteUint32(ofile, (unsigned long)mElements.size());
   
   elt = mElements.begin();
   while (elt != mElements.end())
   {
      WriteString(ofile, elt->first);
      WriteUint32(ofile, (unsigned long)baseOff);
      baseOff += elt->second->getByteSize();
      ++elt;
   }
   
   elt = mElements.begin();
   while (elt != mElements.end())
   {
      elt->second->writeHeader(ofile);
      elt->second->write(ofile);
      ++elt;
   }
}

// write file version 0.2
void BCFile::write_0_2(std::ofstream &ofile, size_t baseOff) const
{
   baseOff += 4; // number of elements
   
   std::map<std::string, BCFileElement*>::const_iterator elt = mElements.begin();
   
   // optimize that by calculating index size as elements are added
   while (elt != mElements.end())
   {
      baseOff += 4; // element name length
      baseOff += elt->first.length() + 1; // element name (includes the \0)
      baseOff += 4; // element data offset in file
      baseOff += 4; // element size
      ++elt;
   }
   
   WriteUint32(ofile, (unsigned long)mElements.size());
   
   size_t sz;
   
   elt = mElements.begin();
   while (elt != mElements.end())
   {
      sz = elt->second->getByteSize();
      WriteString(ofile, elt->first);
      WriteUint32(ofile, (unsigned long)baseOff);
      WriteUint32(ofile, (unsigned long)sz);
      baseOff += sz;
      ++elt;
   }
   
   elt = mElements.begin();
   while (elt != mElements.end())
   {
      elt->second->writeHeader(ofile);
      elt->second->write(ofile);
      ++elt;
   }
}

// read file version 0.1 TOC
bool BCFile::read_0_1(std::ifstream &ifile)
{
   // if any ElementPlaceHolder -> delete them
   clearElements();
   
   unsigned long nelems = 0;
   if (!ReadUint32(ifile, nelems) || ifile.eof())
   {
      return false;
   }
   
   unsigned long off;
   
   for (unsigned long i=0; i<nelems; ++i)
   {
      std::string name; // ReadString if dubious
      
      if (ifile.eof())
      {
         return false;
      }
      
      if (!ReadString(ifile, name) || ifile.eof())
      {
         return false;
      }
      
      if (!ReadUint32(ifile, off))
      {
         return false;
      }
      
      mPlaceHolders.push_back(new ElementPlaceHolder(off));
      mElements[name] = mPlaceHolders.back();
   }
     
   return true;
}

// read file version 0.2 TOC
bool BCFile::read_0_2(std::ifstream &ifile)
{
   // if any ElementPlaceHolder -> delete them
   clearElements();
   
   unsigned long nelems = 0;
   if (!ReadUint32(ifile, nelems) || ifile.eof())
   {
      return false;
   }
   
   unsigned long off, sz;
   
   for (unsigned long i=0; i<nelems; ++i)
   {
      std::string name; // ReadString if dubious
      
      if (ifile.eof())
      {
         return false;
      }
      
      if (!ReadString(ifile, name) || ifile.eof())
      {
         return false;
      }
      
      if (!ReadUint32(ifile, off))
      {
         return false;
      }
      
      if (!ReadUint32(ifile, sz))
      {
         return false;
      }
      
      mPlaceHolders.push_back(new ElementPlaceHolder(off, sz));
      mElements[name] = mPlaceHolders.back();
   }
     
   return true;
}

}


