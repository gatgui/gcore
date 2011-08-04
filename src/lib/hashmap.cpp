#include <gcore/hashmap.h>

namespace gcore
{

unsigned int hash_djb(const unsigned char *data, size_t len)
{
  unsigned int hash = 5381;
  
  for (size_t i=0; i<len; ++i)
  {
    hash = ((hash << 5) + hash) + (unsigned int)data[i]; /* hash * 33 + c */
  }

  return hash;
}

unsigned int hash_jenkins(const unsigned char *data, size_t len)
{
  unsigned int hash = 0;
  
  for (size_t i=0; i<len; ++i)
  {
    hash += (unsigned int) data[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }
  
  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);
  
  return hash;
}

unsigned int hash_fnv1(const unsigned char *data, size_t len)
{
  unsigned int hash = 2166136261; // use 14695981039346656037 for 64 bit hash
  
  for (size_t i=0; i<len; ++i)
  {
    hash = hash * 16777619; // use 1099511628211 for 64 bit hash
    hash = hash ^ ((unsigned int) data[i]);
  }
  
  return hash;
}

unsigned int hash_fnv1a(const unsigned char *data, size_t len)
{
  unsigned int hash = 2166136261; // use 14695981039346656037 for 64 bit hash
  
  for (size_t i=0; i<len; ++i)
  {
    hash = hash ^ ((unsigned int) data[i]);
    hash = hash * 16777619; // use 1099511628211 for 64 bit hash
  }
  
  return hash;
}

unsigned int hash_murmur2(const unsigned char *data_in, size_t len)
{  
  unsigned int seed = 0;
  
  // 'm' and 'r' are mixing constants generated offline.
  // They're not really 'magic', they just happen to work well.
  const unsigned int m = 0x5bd1e995;
  const int r = 24;
  
  // Initialize the hash to a 'random' value
  unsigned int h = seed ^ len;
  
  // Mix 4 bytes at a time into the hash
  const unsigned char *data = data_in;
  
  while (len >= 4)
  {
    unsigned int k = *((unsigned int *) data);
    
    k *= m; 
    k ^= k >> r; 
    k *= m; 
    
    h *= m; 
    h ^= k;
    
    data += 4;
    len -= 4;
  }
  
  // Handle the last few bytes of the input array
  switch (len)
  {
    case 3  : h ^= data[2] << 16;
    case 2  : h ^= data[1] << 8;
    case 1  : h ^= data[0];
              h *= m;
    default : break;
  };
  
  // Do a few final mixes of the hash to ensure the last few
  // bytes are well-incorporated.
  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;
  
  return h;
} 

#define mmix(h,k) { k *= m; k ^= k >> r; k *= m; h *= m; h ^= k; }

unsigned int hash_murmur2a(const unsigned char *data_in, size_t len)
{  
  unsigned int seed = 0;
  
  const unsigned int m = 0x5bd1e995;
  const int r = 24;
  unsigned int l = len;
  
  const unsigned char * data = data_in;
  
  unsigned int h = seed;
  
  while (len >= 4)
  {
    unsigned int k = *(unsigned int*)data;
    
    mmix(h, k);
    
    data += 4;
    len -= 4;
  }
  
  unsigned int t = 0;
  
  switch (len)
  {
    case 3  : t ^= data[2] << 16;
    case 2  : t ^= data[1] << 8;
    case 1  : t ^= data[0];
    default : break;
  };
  
  mmix(h, t);
  mmix(h, l);
  
  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;
  
  return h;
}

}

