/*
MIT License

Copyright (c) 2011 Gaetan Guidet

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

#ifndef __gcore_hashmap_h_
#define __gcore_hashmap_h_

#include <gcore/list.h>

namespace gcore
{
  typedef unsigned int (*HashFunc)(const unsigned char *, size_t);
  
  template <typename T, HashFunc H>
  struct HashValue
  {
    static unsigned int Compute(const T &val)
    {
      return H((const unsigned char *)&val, sizeof(T));
    }
  };

  template <HashFunc H>
  struct HashValue<char*, H>
  {
    static unsigned int Compute(const char* &val)
    {
      return H((const unsigned char *)val, strlen(val));
    }
  };

  template <HashFunc H>
  struct HashValue<std::string, H>
  {
    static unsigned int Compute(const std::string &val)
    {
      return H((const unsigned char *)val.c_str(), val.length());
    }
  };

  GCORE_API unsigned int hash_djb(const unsigned char *data, size_t len);
  GCORE_API unsigned int hash_jenkins(const unsigned char *data, size_t len);
  GCORE_API unsigned int hash_fnv1(const unsigned char *data, size_t len);
  GCORE_API unsigned int hash_fnv1a(const unsigned char *data, size_t len);
  GCORE_API unsigned int hash_murmur2(const unsigned char *data, size_t len);
  GCORE_API unsigned int hash_murmur2a(const unsigned char *data, size_t len);
  
  // ---
  
  template <typename KeyType, typename ValueType, HashFunc H = hash_jenkins>
  class HashMap
  {
  public:
    
    class KeyError : public std::runtime_error
    {
    public:
      explicit KeyError(const std::string &s) throw();
      virtual ~KeyError() throw();
    };

    struct Entry
    {
      KeyType key;
      ValueType value;
      unsigned int h;
      
      bool operator==(const Entry &rhs) const;
    };
    
    typedef typename gcore::List<KeyType> KeyVector;
    typedef typename gcore::List<ValueType> ValueVector;
    
  protected:
    
    typedef typename std::list<Entry> EntryList;
    typedef typename std::vector<EntryList> EntryListVector;
    typedef HashMap<KeyType, ValueType, H> Type;

  public:
    
    class iterator
    {
    public:
      
      friend class HashMap;
      //friend class const_iterator;
      
      iterator();
      iterator(const iterator &rhs);
      ~iterator();
      
      iterator& operator=(const iterator &rhs);
      bool operator==(const iterator &rhs) const;
      bool operator!=(const iterator &rhs) const;
      iterator& operator++();
      iterator operator++(int);
      
      const ValueType& value() const;
      const KeyType& key() const;
      void value(const ValueType &val);
      
      Type* hash() const;
      long bucket() const;
      const typename EntryList::iterator& entry() const;
      
    private:
      
      iterator(Type *h);
      iterator(Type *h, long b, typename EntryList::iterator e);
      
      Type *mHash;
      long mBucket;
      typename EntryList::iterator mEntry;
    };
    
    class const_iterator
    {
    public:
      
      friend class HashMap;
      
      const_iterator();
      const_iterator(const iterator &rhs);
      const_iterator(const const_iterator &rhs);
      ~const_iterator();
      
      const_iterator& operator=(const const_iterator &rhs);
      const_iterator& operator=(const iterator &rhs);
      bool operator==(const const_iterator &rhs) const;
      bool operator==(const iterator &rhs) const;
      bool operator!=(const const_iterator &rhs) const;
      bool operator!=(const iterator &rhs) const;
      const_iterator& operator++();
      const_iterator operator++(int);
      
      const ValueType& value() const;
      const KeyType& key() const;
      
      const Type* hash() const;
      long bucket() const;
      const typename EntryList::const_iterator& entry() const;
      
    private:
      
      const_iterator(const Type *h);
      const_iterator(const Type *h, long b, typename EntryList::const_iterator e);
      
      const Type *mHash;
      long mBucket;
      typename EntryList::const_iterator mEntry;
    };
    
    friend class iterator;
    friend class const_iterator;
    
    // NOTE: iterators are invalidated by HashMap modifications
    
  public:
    
    HashMap(size_t numBuckets=16);
    HashMap(const HashMap<KeyType, ValueType, H> &rhs);
    virtual ~HashMap();
    
    HashMap<KeyType, ValueType, H>& operator=(const HashMap<KeyType, ValueType, H> &rhs);
    // May throw KeyError exception
    const ValueType& operator[](const KeyType &k) const;
    ValueType& operator[](const KeyType &k);
    
    double loadFactor() const;
    double maxLoadFactor() const;
    void setMaxLoadFactor(double lf);
    
    size_t size() const;
    void clear();
    bool has(const KeyType &k) const;
    iterator begin();
    iterator end();
    iterator find(const KeyType &key);
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator find(const KeyType &key) const;
    bool insert(const KeyType &key, const ValueType &val, bool overwrite=false);
    void erase(const KeyType &k);
    void erase(iterator it);
    size_t keys(KeyVector &kl) const;
    size_t values(ValueVector &vl) const;
    
  protected:
    
    void rehash();
    
  protected:
    
    EntryListVector mBuckets;
    size_t mNumBuckets;
    size_t mNumEntries;
    double mMaxLoadFactor;
  };
}

// ---

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline bool gcore::HashMap<KeyType, ValueType, H>::Entry::operator==(const Entry &rhs) const
{
  return (h == rhs.h && key == rhs.key);
}

// ---

template <typename KeyType, typename ValueType, gcore::HashFunc H>
gcore::HashMap<KeyType, ValueType, H>::KeyError::KeyError(const std::string &s) throw()
  : std::runtime_error(std::string("KeyError: ")+s)
{
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
gcore::HashMap<KeyType, ValueType, H>::KeyError::~KeyError() throw()
{
}

// ---

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline gcore::HashMap<KeyType, ValueType, H>::iterator::iterator()
  : mHash(0), mBucket(-1)
{
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline gcore::HashMap<KeyType, ValueType, H>::iterator::iterator(gcore::HashMap<KeyType, ValueType, H> *h)
  : mHash(h), mBucket(long(h->mNumBuckets))
{
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline gcore::HashMap<KeyType, ValueType, H>::iterator::iterator(gcore::HashMap<KeyType, ValueType, H> *h, long b, typename gcore::HashMap<KeyType, ValueType, H>::EntryList::iterator e)
  : mHash(h), mBucket(b), mEntry(e)
{
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline gcore::HashMap<KeyType, ValueType, H>::iterator::iterator(const typename gcore::HashMap<KeyType, ValueType, H>::iterator &rhs)
  : mHash(rhs.mHash), mBucket(rhs.mBucket), mEntry(rhs.mEntry)
{
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline gcore::HashMap<KeyType, ValueType, H>::iterator::~iterator()
{
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
gcore::HashMap<KeyType, ValueType, H>*
gcore::HashMap<KeyType, ValueType, H>::iterator::hash() const
{
  return mHash;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
long gcore::HashMap<KeyType, ValueType, H>::iterator::bucket() const
{
  return mBucket;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
const typename gcore::HashMap<KeyType, ValueType, H>::EntryList::iterator&
gcore::HashMap<KeyType, ValueType, H>::iterator::entry() const
{
  return mEntry;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline typename gcore::HashMap<KeyType, ValueType, H>::iterator&
gcore::HashMap<KeyType, ValueType, H>::iterator::operator=(const typename gcore::HashMap<KeyType, ValueType, H>::iterator &rhs)
{
  mHash = rhs.mHash;
  mBucket = rhs.mBucket;
  mEntry = rhs.mEntry;
  return *this;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline bool gcore::HashMap<KeyType, ValueType, H>::iterator::operator==(const typename gcore::HashMap<KeyType, ValueType, H>::iterator &rhs) const
{
  if (mHash == rhs.mHash && mBucket == rhs.mBucket)
  {
    return (mBucket < 0 || mBucket >= long(mHash->mNumBuckets) ? true : mEntry == rhs.mEntry);
  }
  return false;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline bool gcore::HashMap<KeyType, ValueType, H>::iterator::operator!=(const typename gcore::HashMap<KeyType, ValueType, H>::iterator &rhs) const
{
  return !operator==(rhs);
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline const ValueType& gcore::HashMap<KeyType, ValueType, H>::iterator::value() const
{
  return mEntry->value;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline const KeyType& gcore::HashMap<KeyType, ValueType, H>::iterator::key() const
{
  return mEntry->key;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline void gcore::HashMap<KeyType, ValueType, H>::iterator::value(const ValueType &val)
{
  mEntry->value = val;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
typename gcore::HashMap<KeyType, ValueType, H>::iterator&
gcore::HashMap<KeyType, ValueType, H>::iterator::operator++()
{
  long nbuckets = long(mHash->mNumBuckets);
  if (mBucket < nbuckets)
  {
    ++mEntry;
    if (mEntry == mHash->mBuckets[mBucket].end())
    {
      ++mBucket;
      while (mBucket < nbuckets && mHash->mBuckets[mBucket].size() == 0)
      {
        ++mBucket;
      }
      if (mBucket < nbuckets)
      {
        mEntry = mHash->mBuckets[mBucket].begin();
      }
    }
  }
  return *this;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
typename gcore::HashMap<KeyType, ValueType, H>::iterator
gcore::HashMap<KeyType, ValueType, H>::iterator::operator++(int)
{
  iterator rv(*this);
  long nbuckets = long(mHash->mNumBuckets);
  if (mBucket < nbuckets)
  {
    ++mEntry;
    if (mEntry == mHash->mBuckets[mBucket].end())
    {
      ++mBucket;
      while (mBucket < nbuckets && mHash->mBuckets[mBucket].size() == 0)
      {
        ++mBucket;
      }
      if (mBucket < nbuckets)
      {
        mEntry = mHash->mBuckets[mBucket].begin();
      }
    }
  }
  return rv;
}

// ---

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline gcore::HashMap<KeyType, ValueType, H>::const_iterator::const_iterator()
  : mHash(0), mBucket(-1)
{
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline gcore::HashMap<KeyType, ValueType, H>::const_iterator::const_iterator(const gcore::HashMap<KeyType, ValueType, H> *h)
  : mHash(h), mBucket(long(h->mNumBuckets))
{
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline gcore::HashMap<KeyType, ValueType, H>::const_iterator::const_iterator(const gcore::HashMap<KeyType, ValueType, H> *h, long b, typename gcore::HashMap<KeyType, ValueType, H>::EntryList::const_iterator e)
  : mHash(h), mBucket(b), mEntry(e)
{
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline gcore::HashMap<KeyType, ValueType, H>::const_iterator::const_iterator(const typename gcore::HashMap<KeyType, ValueType, H>::const_iterator &rhs)
  : mHash(rhs.mHash), mBucket(rhs.mBucket), mEntry(rhs.mEntry)
{
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline gcore::HashMap<KeyType, ValueType, H>::const_iterator::const_iterator(const typename gcore::HashMap<KeyType, ValueType, H>::iterator &rhs)
  : mHash(rhs.hash()), mBucket(rhs.bucket()), mEntry(rhs.entry())
{
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline gcore::HashMap<KeyType, ValueType, H>::const_iterator::~const_iterator()
{
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
const gcore::HashMap<KeyType, ValueType, H>*
gcore::HashMap<KeyType, ValueType, H>::const_iterator::hash() const
{
  return mHash;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
long gcore::HashMap<KeyType, ValueType, H>::const_iterator::bucket() const
{
  return mBucket;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
const typename gcore::HashMap<KeyType, ValueType, H>::EntryList::const_iterator&
gcore::HashMap<KeyType, ValueType, H>::const_iterator::entry() const
{
  return mEntry;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline typename gcore::HashMap<KeyType, ValueType, H>::const_iterator&
gcore::HashMap<KeyType, ValueType, H>::const_iterator::operator=(const typename gcore::HashMap<KeyType, ValueType, H>::const_iterator &rhs)
{
  mHash = rhs.mHash;
  mBucket = rhs.mBucket;
  mEntry = rhs.mEntry;
  return *this;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline typename gcore::HashMap<KeyType, ValueType, H>::const_iterator&
gcore::HashMap<KeyType, ValueType, H>::const_iterator::operator=(const typename gcore::HashMap<KeyType, ValueType, H>::iterator &rhs)
{
  mHash = rhs.hash();
  mBucket = rhs.bucket();
  mEntry = rhs.entry();
  return *this;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline bool gcore::HashMap<KeyType, ValueType, H>::const_iterator::operator==(const typename gcore::HashMap<KeyType, ValueType, H>::const_iterator &rhs) const
{
  if (mHash == rhs.mHash && mBucket == rhs.mBucket)
  {
    return (mBucket < 0 || mBucket >= long(mHash->mNumBuckets) ? true : mEntry == rhs.mEntry);
  }
  return false;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline bool gcore::HashMap<KeyType, ValueType, H>::const_iterator::operator==(const typename gcore::HashMap<KeyType, ValueType, H>::iterator &rhs) const
{
  if (mHash == rhs.hash() && mBucket == rhs.bucket())
  {
    return (mBucket < 0 || mBucket >= long(mHash->mNumBuckets) ? true : mEntry == rhs.entry());
  }
  return false;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline bool gcore::HashMap<KeyType, ValueType, H>::const_iterator::operator!=(const typename gcore::HashMap<KeyType, ValueType, H>::const_iterator &rhs) const
{
  return !operator==(rhs);
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline bool gcore::HashMap<KeyType, ValueType, H>::const_iterator::operator!=(const typename gcore::HashMap<KeyType, ValueType, H>::iterator &rhs) const
{
  return !operator==(rhs);
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline const ValueType& gcore::HashMap<KeyType, ValueType, H>::const_iterator::value() const
{
  return mEntry->value;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline const KeyType& gcore::HashMap<KeyType, ValueType, H>::const_iterator::key() const
{
  return mEntry->key;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
typename gcore::HashMap<KeyType, ValueType, H>::const_iterator&
gcore::HashMap<KeyType, ValueType, H>::const_iterator::operator++()
{
  long nbuckets = long(mHash->mNumBuckets);
  if (mBucket < nbuckets)
  {
    ++mEntry;
    if (mEntry == mHash->mBuckets[mBucket].end())
    {
      ++mBucket;
      while (mBucket < nbuckets && mHash->mBuckets[mBucket].size() == 0)
      {
        ++mBucket;
      }
      if (mBucket < nbuckets)
      {
        mEntry = mHash->mBuckets[mBucket].begin();
      }
    }
  }
  return *this;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
typename gcore::HashMap<KeyType, ValueType, H>::const_iterator
gcore::HashMap<KeyType, ValueType, H>::const_iterator::operator++(int)
{
  iterator rv(*this);
  long nbuckets = long(mHash->mNumBuckets);
  if (mBucket < nbuckets)
  {
    ++mEntry;
    if (mEntry == mHash->mBuckets[mBucket].end())
    {
      ++mBucket;
      while (mBucket < nbuckets && mHash->mBuckets[mBucket].size() == 0)
      {
        ++mBucket;
      }
      if (mBucket < nbuckets)
      {
        mEntry = mHash->mBuckets[mBucket].begin();
      }
    }
  }
  return rv;
}

// ---

template <typename KeyType, typename ValueType, gcore::HashFunc H>
gcore::HashMap<KeyType, ValueType, H>::HashMap(size_t numBuckets)
  : mNumBuckets(numBuckets),
    mNumEntries(0),
    mMaxLoadFactor(0.75)
{
  mBuckets.resize(mNumBuckets);
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
gcore::HashMap<KeyType, ValueType, H>::HashMap(const gcore::HashMap<KeyType, ValueType, H> &rhs)
  : mBuckets(rhs.mBuckets),
    mNumBuckets(rhs.mNumBuckets),
    mNumEntries(rhs.mNumEntries),
    mMaxLoadFactor(rhs.mMaxLoadFactor)
{
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
gcore::HashMap<KeyType, ValueType, H>::~HashMap()
{
  mBuckets.clear();
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
gcore::HashMap<KeyType, ValueType, H>&
gcore::HashMap<KeyType, ValueType, H>::operator=(const gcore::HashMap<KeyType, ValueType, H> &rhs)
{
  if (this != &rhs)
  {
    mBuckets = rhs.mBuckets;
    mNumEntries = rhs.mNumEntries;
    mNumBuckets = rhs.mNumBuckets;
    mMaxLoadFactor = rhs.mMaxLoadFactor;
  }
  return *this;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline size_t gcore::HashMap<KeyType, ValueType, H>::size() const
{
  return mNumEntries;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline double gcore::HashMap<KeyType, ValueType, H>::loadFactor() const
{
  return (double(mNumEntries) / double(mNumBuckets));
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline double gcore::HashMap<KeyType, ValueType, H>::maxLoadFactor() const
{
  return mMaxLoadFactor;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline void gcore::HashMap<KeyType, ValueType, H>::setMaxLoadFactor(double lf)
{
  mMaxLoadFactor = lf;
  if (mNumEntries > mMaxLoadFactor * mNumBuckets)
  {
    rehash();
  }
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
void gcore::HashMap<KeyType, ValueType, H>::clear()
{
  mNumEntries = 0;
  for (size_t i=0; i<mBuckets.size(); ++i)
  {
    mBuckets[i].clear();
  }
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
bool gcore::HashMap<KeyType, ValueType, H>::has(const KeyType &k) const
{
  Entry e;
  e.h = HashValue<KeyType, H>::Compute(k);
  e.key = k;
  unsigned int idx = (e.h % mNumBuckets);
  const EntryList &el = mBuckets[idx];
  typename EntryList::const_iterator it = std::find(el.begin(), el.end(), e);
  return (it != el.end());
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
typename gcore::HashMap<KeyType, ValueType, H>::iterator
gcore::HashMap<KeyType, ValueType, H>::begin()
{
  for (size_t i=0; i<mNumBuckets; ++i)
  {
    EntryList &el = mBuckets[i];
    if (el.size() > 0)
    {
      return iterator(this, long(i), el.begin());
    }
  }
  return iterator(this);
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline typename gcore::HashMap<KeyType, ValueType, H>::iterator
gcore::HashMap<KeyType, ValueType, H>::end()
{
  return iterator(this);
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
typename gcore::HashMap<KeyType, ValueType, H>::iterator
gcore::HashMap<KeyType, ValueType, H>::find(const KeyType &key)
{
  Entry e;
  e.h = HashValue<KeyType, H>::Compute(key);
  e.key = key;
  size_t idx = (e.h % mNumBuckets);
  EntryList &el = mBuckets[idx];
  typename EntryList::iterator it = std::find(el.begin(), el.end(), e);
  if (it == el.end())
  {
    return iterator(this);
  }
  else
  {
    return iterator(this, long(idx), it);
  }
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
typename gcore::HashMap<KeyType, ValueType, H>::const_iterator
gcore::HashMap<KeyType, ValueType, H>::begin() const
{
  for (size_t i=0; i<mNumBuckets; ++i)
  {
    const EntryList &el = mBuckets[i];
    if (el.size() > 0)
    {
      return const_iterator(this, long(i), el.begin());
    }
  }
  return const_iterator(this);
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
inline typename gcore::HashMap<KeyType, ValueType, H>::const_iterator
gcore::HashMap<KeyType, ValueType, H>::end() const
{
  return const_iterator(this);
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
typename gcore::HashMap<KeyType, ValueType, H>::const_iterator
gcore::HashMap<KeyType, ValueType, H>::find(const KeyType &key) const
{
  Entry e;
  e.h = HashValue<KeyType, H>::Compute(key);
  e.key = key;
  unsigned int idx = (e.h % mNumBuckets);
  const EntryList &el = mBuckets[idx];
  typename EntryList::const_iterator it = std::find(el.begin(), el.end(), e);
  if (it == el.end())
  {
    return const_iterator(this);
  }
  else
  {
    return const_iterator(this, long(idx), it);
  }
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
bool gcore::HashMap<KeyType, ValueType, H>::insert(const KeyType &key, const ValueType &val, bool overwrite)
{
  Entry e;
  e.h = HashValue<KeyType, H>::Compute(key);
  e.key = key;
  size_t idx = (e.h % mNumBuckets);
  EntryList &el = mBuckets[idx];
  typename EntryList::iterator it = std::find(el.begin(), el.end(), e);
  if (it == el.end())
  {
    el.push_back(e);
    el.back().value = val;
    ++mNumEntries;
    if (mNumEntries > mMaxLoadFactor * mNumBuckets)
    {
      // load factor above 50%
      rehash();
    }
    return true;
  }
  else
  {
    if (overwrite)
    {
      it->value = val;
    }
    return false;
  }
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
void gcore::HashMap<KeyType, ValueType, H>::erase(const KeyType &k)
{
  Entry e;
  e.h = HashValue<KeyType, H>::Compute(k);
  e.key = k;
  unsigned int idx = (e.h % mNumBuckets);
  EntryList &el = mBuckets[idx];
  typename EntryList::iterator it = std::find(el.begin(), el.end(), e);
  if (it != el.end())
  {
    el.erase(it);
    --mNumEntries;
  }
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
void gcore::HashMap<KeyType, ValueType, H>::erase(typename gcore::HashMap<KeyType, ValueType, H>::iterator it)
{
  if (it.hash == this && it.bucket >= 0 && it.bucket < long(mNumBuckets))
  {
    EntryList &el = mBuckets[it.bucket];
    el.erase(it.entry);
    --mNumEntries;
  }
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
const ValueType& gcore::HashMap<KeyType, ValueType, H>::operator[](const KeyType &k) const
{
  Entry e;
  e.h = HashValue<KeyType, H>::Compute(k);
  e.key = k;
  unsigned int idx = (e.h % mNumBuckets);
  const EntryList &el = mBuckets[idx];
  typename EntryList::const_iterator it = std::find(el.begin(), el.end(), e);
  if (it == el.end())
  {
    throw KeyError("");
  }
  return it->value;
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
ValueType& gcore::HashMap<KeyType, ValueType, H>::operator[](const KeyType &k)
{
  Entry e;
  e.h = HashValue<KeyType, H>::Compute(k);
  e.key = k;
  size_t idx = (e.h % mNumBuckets);
  EntryList &el = mBuckets[idx];
  typename EntryList::iterator it = std::find(el.begin(), el.end(), e);
  if (it == el.end())
  {
    el.push_back(e);
    ++mNumEntries;
    if (mNumEntries > mMaxLoadFactor * mNumBuckets)
    {
      rehash();
      idx = (e.h % mNumBuckets);
      EntryList &el2 = mBuckets[idx];
      return std::find(el2.begin(), el2.end(), e)->value;
    }
    else
    {
      return el.back().value;
    }
  }
  else
  {
    return it->value;
  }
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
size_t gcore::HashMap<KeyType, ValueType, H>::keys(typename gcore::HashMap<KeyType, ValueType, H>::KeyVector &kl) const
{
  kl.resize(mNumEntries);
  for (size_t i=0, j=0; i<mNumBuckets; ++i)
  {
    const EntryList &el = mBuckets[i];
    typename EntryList::const_iterator it = el.begin();
    while (it != el.end())
    {
      kl[j] = it->key;
      ++j;
      ++it;
    }
  }
  return kl.size();
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
size_t gcore::HashMap<KeyType, ValueType, H>::values(typename gcore::HashMap<KeyType, ValueType, H>::ValueVector &vl) const
{
  vl.resize(mNumEntries);
  for (size_t i=0, j=0; i<mNumBuckets; ++i)
  {
    const EntryList &el = mBuckets[i];
    typename EntryList::const_iterator it = el.begin();
    while (it != el.end())
    {
      vl[j] = it->value;
      ++j;
      ++it;
    }
  }
  return vl.size();
}

template <typename KeyType, typename ValueType, gcore::HashFunc H>
void gcore::HashMap<KeyType, ValueType, H>::rehash()
{
  size_t oNumBuckets = mNumBuckets;
  mNumBuckets *= 2;
  EntryListVector buckets(mNumBuckets);
  std::swap(buckets, mBuckets);
  for (size_t i=0; i<oNumBuckets; ++i)
  {
    EntryList &el = buckets[i];
    typename EntryList::iterator it = el.begin();
    while (it != el.end())
    {
      size_t idx = (it->h % mNumBuckets);
      mBuckets[idx].push_back(*it);
      ++it;
    }
  }
}

#endif
