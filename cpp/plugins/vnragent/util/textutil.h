#pragma once

// textutil.h
// 5/7/2014 jichi

#include <QtCore/QString>

namespace Util {

///  Assume the text is Japanese, and return if it is not understandable for non-Japanese speakers.
bool needsTranslation(const QString &text);

inline bool allAscii(const char *text)
{
  while (*text)
    if ((signed char)*text++ < 0)
      return false;
  return true;
}

inline bool allAscii(const wchar_t *text)
{
  while (*text)
    if (*text++ >= 128)
      return false;
  return true;
}

inline bool allAscii(const QString &text)
{ return allAscii(static_cast<const wchar_t *>(text.utf16())); }

} // namespace Util

// EOF
