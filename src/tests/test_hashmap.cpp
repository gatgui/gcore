#include <gcore/hashmap.h>
#include <iostream>
#include <algorithm>
#include <set>
#include <map>
#include <cmath>
#include <cassert>
#include <ctime>

#define MAX_STRING_LENGTH 512

//const size_t  MaxStringLength = 512;
const char   *ValidChars      = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";
const size_t  NumValidChars   = strlen(ValidChars);

inline int RandomNumber(int minval, int maxval)
{
  return minval + rand() % (maxval - minval + 1);
}

std::string RandomString()
{
  std::string s;
  //size_t len = rand() % MaxStringLength;
  size_t len = RandomNumber(5, 30);
  for (size_t i=0; i<len; ++i)
  {
    size_t ic = rand() % NumValidChars;
    s.push_back(ValidChars[ic]);
  }
  return s;
}

using namespace gcore;

int main(int argc, char **argv)
{
  size_t nkeys = (1 << 16);
  size_t nlookup = 100000;
  double lf = 0.75;
  
  if (argc >= 2)
  {
    int n;
    if (sscanf(argv[1], "%d", &n) == 1)
    {
      std::cout << "Use " << n << " keys" << std::endl;
      nkeys = size_t(n);
    }
  }
  if (argc >= 3)
  {
    int l;
    if (sscanf(argv[2], "%d", &l) == 1)
    {
      std::cout << "Do " << l << " lookups" << std::endl;
      nlookup = size_t(l);
    }
  }
  if (argc >= 4)
  {
    double d;
    if (sscanf(argv[3], "%lf", &d) == 1)
    {
      std::cout << "Use max load factor: " << d << std::endl;
      lf = d;
    }
  }
  
  size_t ndups = 0;
  std::cout << "Testing insertion and search with " << nkeys << " keys" << std::endl;
  
  HashMap<std::string, std::string> hmap;
  
  hmap.setMaxLoadFactor(lf);
  
  std::vector<std::string> allKeys;
  
  allKeys.resize(nkeys);
  
  for (size_t i=0; i<nkeys; ++i)
  {
    std::string s = RandomString();
    allKeys[i] = s;
    if (!hmap.insert(s, s))
    {
      std::cout << "Duplicate key found: \""  << s << "\"" << std::endl;
      ++ndups;
    }
  }
  
  std::cout << hmap.size() << " entries (" << ndups << " keys were duplicate)" << std::endl;
  std::cout << "Load Factor of " << hmap.loadFactor() << std::endl;
  
  for (size_t i=0; i<nkeys; ++i)
  {
    size_t ik = rand() % nkeys;
    const std::string &k = allKeys[ik];
    const std::string &v = hmap[k];
    if (v != k)
    {
      std::cerr << "key/value mistmatch" << std::endl;
    }
  }
  
  HashMap<std::string, std::string>::KeyVector keys;
  HashMap<std::string, std::string>::ValueVector vals;
  
  hmap.getKeys(keys);
  hmap.getValues(vals);
  
  assert(hmap.size() == keys.size());
  assert(hmap.size() == vals.size());
  
  for (size_t i=0; i<hmap.size(); ++i)
  {
    if (keys[i] != vals[i])
    {
      std::cerr << "key/value pair mismatch" << std::endl;
    }
  }
  
  {
    HashMap<std::string, std::string> hmap;
    std::map<std::string, std::string> smap;
    size_t numhits = 0;
    
    hmap.setMaxLoadFactor(lf);
    
    std::vector<std::string> allKeys, extraKeys;
    std::string val;
    
    allKeys.resize(nkeys);
    extraKeys.resize(nkeys);
    
    for (size_t i=0; i<nkeys; ++i)
    {
      allKeys[i] = RandomString();
      extraKeys[i] = RandomString();
    }
    
    clock_t st, et;
    
    std::cout << "Insert " << nkeys << " values in HashMap...";
    st = clock();
    for (size_t i=0; i<nkeys; ++i)
    {
      std::string &s = allKeys[i];
      hmap.insert(s, s);
    }
    et = clock();
    std::cout << double(et - st) / CLOCKS_PER_SEC << "(s)" << std::endl;
    
    std::cout << "Do " << nlookup << " random access in HashMap ";
    HashMap<std::string, std::string>::iterator hit;
    st = clock();
    numhits = 0;
    for (size_t i=0; i<nlookup; ++i)
    {
      int keyi = RandomNumber(0, int(nkeys)-1);
      std::string &k = (RandomNumber(0, 1) == 0 ? allKeys[keyi] : extraKeys[keyi]);
      hit = hmap.find(k);
      if (hit != hmap.end())
      {
        ++numhits;
        val = hit.value();
      }
    }
    et = clock();
    std::cout << double(et - st) / CLOCKS_PER_SEC << "(s) [" << numhits << " hit(s)]" << std::endl;
    
    std::cout << "Insert " << nkeys << " values in std::map...";
    st = clock();
    for (size_t i=0; i<nkeys; ++i)
    {
      std::string &s = allKeys[i];
      smap[s] = s;
    }
    et = clock();
    std::cout << double(et - st) / CLOCKS_PER_SEC << "(s)" << std::endl;
    
    std::cout << "Do " << nlookup << " random access in std::map ";
    std::map<std::string, std::string>::iterator it;
    st = clock();
    numhits = 0;
    for (size_t i=0; i<nlookup; ++i)
    {
      int keyi = RandomNumber(0, int(nkeys)-1);
      std::string &k = (RandomNumber(0, 1) == 0 ? allKeys[keyi] : extraKeys[keyi]);
      it = smap.find(k);
      if (it != smap.end())
      {
        ++numhits;
        val = it->second;
      }
    }
    et = clock();
    std::cout << double(et - st) / CLOCKS_PER_SEC << "(s) [" << numhits << " hit(s)]" << std::endl;
    
    
    std::cout << "Check HashMap consistency... ";
    bool consistent = true;
    for (size_t i=0; i<nkeys; ++i)
    {
      int keyi = RandomNumber(0, int(nkeys)-1);
      std::string &k = (RandomNumber(0, 1) == 0 ? allKeys[keyi] : extraKeys[keyi]);
      hit = hmap.find(k);
      it = smap.find(k);
      if (it == smap.end())
      {
        if (hit != hmap.end())
        {
          consistent = false;
          break;
        }
      }
      else
      {
        if (hit == hmap.end())
        {
          consistent = false;
          break;
        }
        if (hit.value() != it->second)
        {
          consistent = false;
          break;
        }
      }
    }
    std::cout << (consistent ? "Yes" : "No") << std::endl;
    
    if (hmap.size() <= 100)
    {
      HashMap<std::string, std::string>::const_iterator cit = hmap.begin();
      while (cit != hmap.end())
      {
        std::cout << cit.key() << " => " << cit.value() << std::endl;
        ++cit;
      }
    }
  }
  
  return 0;
}
