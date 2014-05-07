// codepage.cc
// 5/6/2014 jichi
#include "util/codepage.h"
#include <QtCore/QString>
//#include <qt_windows.h>

uint Util::codePageForEncoding(const QString &encoding)
{
  QString t = encoding.toLower();
  if (t == ENC_SJIS)
    return SjisCodePage;
  if (t == ENC_GBK)
    return GbkCodePage;
  if (t == ENC_BIG5)
    return Big5CodePage;
  if (t == ENC_KSC)
    return KscCodePage;
  if (t == ENC_UTF8)
    return Utf8CodePage;
  if (t == ENC_UTF16)
    return Utf16CodePage;
  return NullCodePage;
}

const char *Util::encodingForCodePage(uint cp)
{
  switch (cp) {
  case SjisCodePage: return ENC_SJIS;
  case GbkCodePage: return ENC_GBK;
  case Big5CodePage: return ENC_BIG5;
  case KscCodePage: return ENC_KSC;
  case Utf8CodePage: return ENC_UTF8;
  case Utf16CodePage: return ENC_UTF16;
  default: return nullptr;
  }
}

// EOF
