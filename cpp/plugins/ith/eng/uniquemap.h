#ifndef UNIQUEMAP_H
#define UNIQUEMAP_H

// uniquemap.h
// 7/25/2014 jichi
// Use machine-learning idear to identify repetive memory access.


/**
 *  Detect unique memory accesses.
 *  - key: text split
 *  - value: text address
 *  The same key is not allowed to repeatly  access the same value.
 *
 *  Use LRU policy to limit the maximum size.
 */
class uniquemap
{
public:
  typedef unsigned long key_type; // DWORD
  typedef unsigned long value_type; // DWORD

  /**
   *  This function is thread-safe.
   *  @param  key  text split
   *  @param  value  text address
   *  @return  whether the value is the same as last value for the specified key.
   */
  bool update(key_type key, value_type value);

  uniquemap(); /// set all fields to zero
private:
  enum { capacity = 0x20 };
  key_type keys_[capacity];
  value_type values_[capacity];
  int next_; // point to next index
};

#endif // UNIQUEMAP_H
