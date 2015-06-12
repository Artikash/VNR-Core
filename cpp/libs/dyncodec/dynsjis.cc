// dynsjis.cc
// 6/11/2015 jichi
// http://en.wikipedia.org/wiki/Shift_JIS
#include "dyncodec/dynsjis.h"

static const char *_CharNextA(const char *s)
{
  if (!s || !s[0])
    return s;
  if (!s[1])
    return s + 1;

  unsigned char ch = static_cast<unsigned char>(s[0]);
  if (ch <= 127 || ch >= 0xa1 && ch <= 0xdf)
    return s + 1;
  return s + 2; // unused byte treated as two-byte character
}

char * stdcall dynsjis::CharNextA(const char *s)
{ return const_cast<char *>(_CharNextA(s)); }

// EOF
