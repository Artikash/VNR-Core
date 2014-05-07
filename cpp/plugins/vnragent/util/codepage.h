#pragma once

// codepage.h
// 5/6/2014 jichi

#include <QtCore/QtGlobal>

QT_FORWARD_DECLARE_CLASS(QString)

namespace Util {

// See: http://msdn.microsoft.com/en-us/library/dd317756%28VS.85%29.aspx
enum CodePage {
  NullCodePage = 0
  , Utf8CodePage = 65001 // UTF-8
  , Utf16CodePage = 1200 // UTF-16
  , SjisCodePage = 932  // SHIFT-JIS
  , GbkCodePage = 936    // GB2312
  , KscCodePage = 949   // KS_C
  , Big5CodePage = 950  // BIG5
};

uint codePageForEncoding(const QString &encoding);

} // namespace Util

// EOF
