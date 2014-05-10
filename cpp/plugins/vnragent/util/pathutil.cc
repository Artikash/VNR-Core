// pathutil.cc
// 5/7/2014 jichi
#include "util/pathutil.h"
#include <cstring>

const char *Util::basename(const char *s)
{
  if (const char *r = ::strrchr(s, path_sep))
    return r + 1; // skip the path seperator
  else
    return s;
}

const wchar_t *Util::basename(const wchar_t *s)
{
  if (const wchar_t *r = ::wcsrchr(s, path_sep))
    return r + 1; // skip the path seperator
  else
    return s;
}

// EOF
