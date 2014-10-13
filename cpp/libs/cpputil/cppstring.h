#ifndef CPPSTRING_H
#define CPPSTRING_H

// cppstring.h
// 10/12/2014 jichi

#include <cstddef> // for size_t

// strlen

template <typename charT>
inline size_t cpp_basic_strlen(const charT *s)
{
  const charT *p = s;
  while (*p) p++;
  return p - s;
}

inline size_t cpp_strlen(const char *s) { return cpp_basic_strlen<char>(s); }
inline size_t cpp_wstrlen(const wchar_t *s) { return cpp_basic_strlen<wchar_t>(s); }

template <typename charT>
inline size_t cpp_basic_strnlen(const charT *s, size_t n)
{
  const charT *p = s;
  while (*p && n) p++, n--;
  return p - s;
}

inline size_t cpp_strnlen(const char *s, size_t n) { return cpp_basic_strnlen<char>(s, n); }
inline size_t cpp_wstrnlen(const wchar_t *s, size_t n) { return cpp_basic_strnlen<wchar_t>(s, n); }

#endif // CPPSTRING_H
