// textutil.cc
// 5/7/2014 jichi
#include "util/textutil.h"
#include <QtCore/QString>

// Currently, only for Japanese text.
// Return if the text contains non-ascii characters.
bool Util::needsTranslation(const QString &text)
{
  foreach (const QChar &c, text)
    if (c.unicode() > 255)
      return true;
  return false;
}

// EOF
