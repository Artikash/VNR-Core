#pragma once

// winutil.h
// 6/13/2013 jichi

#ifdef _MSC_VER
# include <cstddef> // for wchar_t
#endif // _MSC_VER

// - Shell32 -

bool winutil_resolve_link(const wchar_t *link, wchar_t *buf, int bufsize);

// EOF
