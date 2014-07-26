// uniquemap.cc
// 7/25/2014 jichi

#include "uniquemap.h"

uniquemap::uniquemap()
  : next_(0) //, keys_{}, values_{}  // not supported by msvc11
{
  for (int i = 0; i < capacity; i++)
    keys_[i] = values_[i] = 0;
}

bool uniquemap::update(key_type key, value_type value)
{
  int next = next_; // backup next to avoid overflow when invoked by multiple threads
  bool ret = true;
  for (int i = 0; i < capacity; i++)
    if (keys_[i] == key) {
      ret = values_[i] != value;
      int prev = (next - 1) % capacity; // LRU policy, cyclic
      if (prev == i) {
        values_[prev] = value;
        return ret;
      }
      break;
    }
  keys_[next] = key;
  values_[next] = value;
  next_ = (next + 1) % capacity; // LRU policy, cyclic
  return ret;
}

// EOF
