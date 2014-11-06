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

// strnchr

#define cpp_basic_strnchr_(s, n, c) \
  { \
    while (*s && n) { \
      if (*s == c) \
        return s; \
      s++, n--; \
    } \
    return nullptr; \
  }
template <typename charT>
inline charT *cpp_basic_strnchr(charT *s, size_t n, charT c) cpp_basic_strnchr_(s, n, c)
template <typename charT>
inline const charT *cpp_basic_strnchr(const charT *s, size_t n, charT c) cpp_basic_strnchr_(s, n, c)

inline char *cpp_strnchr(char *s, size_t n, char c) { return cpp_basic_strnchr<char>(s, n, c); }
inline const char *cpp_strnchr(const char *s, size_t n, char c) { return cpp_basic_strnchr<char>(s, n, c); }
inline wchar_t *cpp_wcsnchr(wchar_t *s, size_t n, wchar_t c) { return cpp_basic_strnchr<wchar_t>(s, n, c); }
inline const wchar_t *cpp_wcsnchr(const wchar_t *s, size_t n, wchar_t c) { return cpp_basic_strnchr<wchar_t>(s, n, c); }

// strnstr

#define cpp_basic_strnstr_(s, n, r, ncmp) \
  { \
    while (*s && n) { \
      if (ncmp(s, r, n) == 0) \
        return s; \
      s++, n--; \
    } \
    return nullptr; \
  }

template <typename charT>
inline charT *cpp_basic_strnstr(charT *s, size_t n, const charT *r) cpp_basic_strnstr_(s, n, r, ::strncmp)
template <typename charT>
inline const charT *cpp_basic_strnstr(const charT *s, size_t n, const charT *r) cpp_basic_strnstr_(s, n, r, ::strncmp)

template <>
inline wchar_t *cpp_basic_strnstr<wchar_t>(wchar_t *s, size_t n, const wchar_t *r) cpp_basic_strnstr_(s, n, r, ::wcsncmp)
template <>
inline const wchar_t *cpp_basic_strnstr<wchar_t>(const wchar_t *s, size_t n, const wchar_t *r) cpp_basic_strnstr_(s, n, r, ::wcsncmp)

inline char *cpp_strnstr(char *s, size_t n, const char *r) { return cpp_basic_strnstr<char>(s, n, r); }
inline const char *cpp_strnstr(const char *s, size_t n, const char *r) { return cpp_basic_strnstr<char>(s, n, r); }
inline wchar_t *cpp_wcsnstr(wchar_t *s, size_t n, const wchar_t *r) { return cpp_basic_strnstr<wchar_t>(s, n, r); }
inline const wchar_t *cpp_wcsnstr(const wchar_t *s, size_t n, const wchar_t *r) { return cpp_basic_strnstr<wchar_t>(s, n, r); }

#endif // CPPSTRING_H
