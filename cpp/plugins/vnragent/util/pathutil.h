#pragma once

// pathtutil.h
// 5/7/2014 jichi

#include <cstddef> // for size_t
#ifdef _MSC_VER
# include <wchar.h> // for wchar_t
#endif // _MSC_VER

namespace Util {

enum { path_sep = '\\' };

// Return basename offset
const char *basename(const char *s);
const wchar_t *basename(const wchar_t *s);

} // namespace Util

// EOF
