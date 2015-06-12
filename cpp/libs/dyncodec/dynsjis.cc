// dynsjis.cc
// 6/11/2015 jichi
// http://en.wikipedia.org/wiki/Shift_JIS
#include "dyncodec/dynsjis.h"

const char *dynsjis::next_char(const char *s)
{
  if (!s || !s[0])
    return s;
  if (!s[1])
    return s + 1;
  if (is_single_char(s[0]))
    return s + 1;
  return s + 2; // unused byte treated as two-byte character
}

const char *dynsjis::prev_char(const char *s, const char *begin)
{
  if (!s || s <= begin)
    return s;
  if (!*s || s == begin + 1)
    return s - 1;
  if (!is_single_char(s[0]))
    return s - 2;
  if (is_single_char(s[-1]))
    return s - 1;
  // 0 is single-width
  // -1 is double-width
  if (is_single_char(s[-3]))
    return s - 2;
  const char *p = s - 1;
  while (p != begin && !is_single_char(*p))
    p--;
  size_t dist = s - p;
  if (is_single_char(*p))
    dist++;
  return s - 1 - (dist % 2);
}

// EOF
