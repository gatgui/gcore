#include <gcore/bcfile.h>
using namespace gcore;

class Vertices : public BCFileElement
{
protected:

   struct Vertex
   {
      float x, y, z;
   };

public:

   Vertices()
   {
   }

   virtual ~Vertices()
   {
      mData.clear();
   }

   void clear()
   {
      mData.clear();
   }

   void add(float x, float y, float z)
   {
      Vertex v = {x, y, z};
      mData.push_back(v);
   }

   void get(size_t idx, float &x, float &y, float &z)
   {
      x = mData[idx].x;
      y = mData[idx].y;
      z = mData[idx].z;
   }

   size_t size() const
   {
      return mData.size();
   }

   virtual size_t getByteSize() const
   {
      size_t sz = (4 + mData.size() * 3 * sizeof(float));
      return sz;
   }

   virtual void writeHeader(std::ostream &) const
   {
      // NOOP
   }

   virtual void write(std::ostream &ofile) const
   {
      WriteUint32(ofile, mData.size());
      for (size_t i=0; i<mData.size(); ++i)
      {
         WriteFloat(ofile, mData[i].x);
         WriteFloat(ofile, mData[i].y);
         WriteFloat(ofile, mData[i].z);
      }
   }

   virtual bool readHeader(std::istream &)
   {
      return true;
   }

   virtual bool read(std::istream &ifile)
   {
      unsigned long len;
      ReadUint32(ifile, len);
      mData.resize(len);
      for (unsigned long i=0; i<len; ++i)
      {
         ReadFloat(ifile, mData[i].x);
         ReadFloat(ifile, mData[i].y);
         ReadFloat(ifile, mData[i].z);
      }
      return true;
   }

   friend std::ostream& operator<<(std::ostream &os, const Vertices &v)
   {
      os << "{";
      if (v.mData.size() > 0)
      {
         size_t i=0;
         for (i=0; i<v.mData.size()-1; ++i)
         {
            os << "(" << v.mData[i].x << ", " << v.mData[i].y << ", " << v.mData[i].z << "), ";
         }
         os << "(" << v.mData[i].x << ", " << v.mData[i].y << ", " << v.mData[i].z << ")";
      }
      os << "}";
      return os;
   }

protected:

   std::vector<Vertex> mData;
};


class Polygons : public BCFileElement
{
protected:

   typedef std::vector<size_t> Polygon;

public:
   
   Polygons()
   {
   }

   virtual ~Polygons()
   {
      mData.clear();
   }

   void clear()
   {
      mData.clear();
   }

   size_t size() const
   {
      return mData.size();
   }

   void add(size_t v0, size_t v1, size_t v2)
   {
      mData.push_back(Polygon());
      mData.back().resize(3);
      mData.back()[0] = v0;
      mData.back()[1] = v1;
      mData.back()[2] = v2;
   }

   void add(size_t v0, size_t v1, size_t v2, size_t v3)
   {
      mData.push_back(Polygon());
      mData.back().resize(4);
      mData.back()[0] = v0;
      mData.back()[1] = v1;
      mData.back()[2] = v2;
      mData.back()[3] = v3;
   }

   virtual size_t getByteSize() const
   {
      size_t sz = 4; // pcount
      for (size_t i=0; i<mData.size(); ++i)
      {
         const Polygon &p = mData[i];
         sz += 4 * (1 + p.size()); // vcount + vindices
      }
      return sz;
   }

   virtual void writeHeader(std::ostream &) const
   {
      // NOOP
   }

   virtual void write(std::ostream &ofile) const
   {
      WriteUint32(ofile, mData.size());
      for (size_t i=0; i<mData.size(); ++i)
      {
         const Polygon &p = mData[i];
         WriteUint32(ofile, p.size());
         for (size_t j=0; j<p.size(); ++j)
         {
            WriteUint32(ofile, p[j]);
         }
      }
   }

   virtual bool readHeader(std::istream &)
   {
      return true;
   }

   virtual bool read(std::istream &ifile)
   {
      unsigned long np, nv, vi;
      ReadUint32(ifile, np);
      mData.resize(np);
      for (unsigned long i=0; i<np; ++i)
      {
         Polygon &p = mData[i];
         ReadUint32(ifile, nv);
         p.resize(nv);
         for (unsigned long j=0; j<nv; ++j)
         {
            ReadUint32(ifile, vi);
            p[j] = vi;
         }
      }
      return true;
   }

   friend std::ostream& operator<<(std::ostream &os, const Polygons &ps)
   {
      os << "{";
      if (ps.mData.size() > 0)
      {
         size_t i=0;
         for (i=0; i<ps.mData.size()-1; ++i)
         {
            const Polygon &p = ps.mData[i];
            os << "(";
            if (p.size() > 0)
            {
               size_t j=0;
               for (j=0; j<p.size()-1; ++j)
               {
                  os << p[j] << ", ";
               }
               os << p[j];
            }
            os << "), ";
         }
         const Polygon &p = ps.mData[i];
         os << "(";
         if (p.size() > 0)
         {
            size_t j=0;
            for (j=0; j<p.size()-1; ++j)
            {
               os << p[j] << ", ";
            }
            os << p[j];
         }
         os << ")";
      }
      os << "}";
      return os;
   }

protected:

   std::vector<Polygon> mData;
};

int main(int, char**)
{
   Vertices verts;
   Polygons polys;

   verts.add(0,0,0);
   verts.add(0,1,0);
   verts.add(1,1,0);
   verts.add(1,0,0);

   polys.add(0,2,1);
   polys.add(0,3,2);

   BCFile fc;

   std::cout << "=== Write to file: " << std::endl;
   std::cout << "  " << verts << std::endl;
   std::cout << "  " << polys << std::endl;

   fc.addElement("vertices", &verts);
   fc.addElement("polygons", &polys);
   fc.write("./test.bfc", false);

   fc.clearElements();

   verts.clear();
   polys.clear();
   std::cout << "=== Clear" << std::endl;
   std::cout << "  " << verts << std::endl;
   std::cout << "  " << polys << std::endl;

   fc.readTOC("./test.bfc");
   std::cout << "has polygons: " << fc.hasElement("polygons") << std::endl;
   std::cout << "has vertices: " << fc.hasElement("vertices") << std::endl;
   std::cout << "read vertices... " << fc.readElement("vertices", &verts) << std::endl;
   std::cout << "read polygons... " << fc.readElement("polygons", &polys) << std::endl;

   std::cout << "=== Read from file: " << std::endl;
   std::cout << "  " << verts << std::endl;
   std::cout << "  " << polys << std::endl;

   return 0;
}


