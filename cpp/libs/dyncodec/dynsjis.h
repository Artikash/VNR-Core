#ifndef DYNSJIS_H
#define DYNSJIS_H

// dynsjis.h
// 6/11/2015 jichi

namespace dynsjis {

static inline bool is_single_byte(unsigned char ch)
{ return ch <= 127 || ch >= 0xa1 && ch <= 0xdf; }

static inline bool is_single_char(unsigned char ch)
{ return is_single_byte(static_cast<unsigned char>(ch)); }

const char *next_char(const char *s);
inline char *next_char(char *s)
{ return const_cast<char *>(next_char(static_cast<const char *>(s))); }

const char *prev_char(const char *s, const char *begin = nullptr);
inline char *prev_char(char *s, const char *begin = nullptr)
{ return const_cast<char *>(prev_char(static_cast<const char *>(s), begin)); }

} // namespace dynsjis

#endif // DYNSJIS_H
