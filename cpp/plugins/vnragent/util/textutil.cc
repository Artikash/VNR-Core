// textutil.cc
// 5/7/2014 jichi
#include "util/textutil.h"
#include "unistr/unichar.h"
#include <QtCore/QString>

// Currently, only for Japanese text.
// Return if the text contains non-ascii characters.
bool Util::needsTranslation(const QString &text) { return !allAscii(text); }

bool Util::allHangul(const wchar_t *s)
{
  for (; *s; s++)
    if (*s > 127 && !unistr::isspace(*s) && !unistr::ishangul(*s) && !unistr::ispunct(*s))
      return false;
  return true;
}

// EOF
