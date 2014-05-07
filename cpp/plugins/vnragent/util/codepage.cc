// codepage.cc
// 5/6/2014 jichi
#include "util/codepage.h"
#include <QtCore/QString>
//#include <qt_windows.h>

uint Util::codePageForEncoding(const QString &encoding)
{
  QString t = encoding.toLower();
  if (t == "shift-jis")
    return SjisCodePage;
  if (t == "gbk")
    return GbkCodePage;
  if (t == "big5")
    return Big5CodePage;
  if (t == "ks-c")
    return KscCodePage;
  if (t == "utf-8")
    return Utf8CodePage;
  if (t == "utf-16")
    return Utf16CodePage;
  return NullCodePage;
}

// EOF
