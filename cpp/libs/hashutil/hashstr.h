#ifndef HASHSTR_H
#define HASHSTR_H

// hashstr.h
// 8/1/2011
// See: http://www.cse.yorku.ca/~oz/hash.html

#include "hashutil/hashutil.h"
#include <cstdint>

HASHUTIL_BEGIN_NAMESPACE

enum : uint64_t { djb2_init = 5381 };

///  djb2: h = h*33 + c
template <typename Tchar>
inline uint64_t djb2(const Tchar *str, uint64_t hash = djb2_init)
{
  Tchar c;
  while ((c = *str++))
    hash = ((hash << 5) + hash) + c; // hash * 33 + c
  return hash;
}

///  n: length
template <typename Tchar>
inline uint64_t djb2_n(const Tchar *str, size_t len, uint64_t hash = djb2_init)
{
  while (len--)
    hash = ((hash << 5) + hash) + (*str++); // hash * 33 + c
  return hash;
}

///  sdbm: hash(i) = hash(i - 1) * 65599 + str[i];
template <typename Tchar>
inline uint64_t sdbm(const Tchar *str, uint64_t hash = 0)
{
  Tchar c;
  while ((c = *str++))
     hash = c + (hash << 6) + (hash << 16) - hash;
  return hash;
}

template <typename Tchar>
inline uint64_t loselose(const Tchar *str, uint64_t hash = 0)
{
  Tchar c;
  while ((c = *str++))
    hash += c;
  return hash;
}

HASHUTIL_END_NAMESPACE

#endif // HASHSTR_H
