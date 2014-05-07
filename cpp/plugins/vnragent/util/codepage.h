#pragma once

// codepage.h
// 5/6/2014 jichi

#include <QtCore/QtGlobal>

QT_FORWARD_DECLARE_CLASS(QString)

#define ENC_UTF8    "utf-8"
#define ENC_UTF16   "utf-16"
#define ENC_SJIS    "shift-jis"
#define ENC_GBK     "gbk"
#define ENC_BIG5    "big5"
#define ENC_KSC     "euc-kr"
#define ENC_KOI8    "koi8-r" // Windows-1251

namespace Util {

// See: http://msdn.microsoft.com/en-us/library/dd317756%28VS.85%29.aspx
enum CodePage {
  NullCodePage = 0
  , Utf8CodePage = 65001 // UTF-8
  , Utf16CodePage = 1200 // UTF-16
  , SjisCodePage = 932  // SHIFT-JIS
  , GbkCodePage = 936   // GB2312
  , KscCodePage = 949   // EUC-KR
  , Big5CodePage = 950  // BIG5
  , Koi8CodePage = 866  // KOI8-R
};

uint codePageForEncoding(const QString &encoding);
const char *encodingForCodePage(uint cp);

quint8 charSetForCodePage(uint cp);

quint8 currentCharSet();

} // namespace Util

// EOF
