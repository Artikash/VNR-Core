// codepage.cc
// 5/6/2014 jichi
#include "util/codepage.h"
#include "qtembedplugin/codecmanager.h"
#include <QtCore/QString>
#include <QtCore/QTextCodec>
#include <qt_windows.h>

QTextCodec *Util::codecForName(const char *name)
{
  QTextCodec *ret = QTextCodec::codecForName(name);
  return ret ? ret : QtEmbedPlugin::CodecManager::globalInstance()->codecForName(name);
}

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
  if (t == ENC_TIS)
    return TisCodePage;
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
  case Koi8CodePage: return ENC_KOI8;
  case Utf8CodePage: return ENC_UTF8;
  case Utf16CodePage: return ENC_UTF16;
  default: return nullptr;
  }
}

// #define ANSI_CHARSET            0
// #define DEFAULT_CHARSET         1
// #define SYMBOL_CHARSET          2
// #define SHIFTJIS_CHARSET        128
// #define HANGEUL_CHARSET         129
// #define HANGUL_CHARSET          129
// #define GB2312_CHARSET          134
// #define CHINESEBIG5_CHARSET     136
// #define OEM_CHARSET             255
// #if(WINVER >= 0x0400)
// #define JOHAB_CHARSET           130
// #define HEBREW_CHARSET          177
// #define ARABIC_CHARSET          178
// #define GREEK_CHARSET           161
// #define TURKISH_CHARSET         162
// #define VIETNAMESE_CHARSET      163
// #define THAI_CHARSET            222
// #define EASTEUROPE_CHARSET      238
// #define RUSSIAN_CHARSET         204

quint8 Util::charSetForCodePage(uint cp)
{
  switch (cp) {
  case TisCodePage:     return THAI_CHARSET;
  case Koi8CodePage:    return RUSSIAN_CHARSET;
  case SjisCodePage:    return SHIFTJIS_CHARSET;
  case GbkCodePage:     return GB2312_CHARSET;
  case Big5CodePage:    return CHINESEBIG5_CHARSET;
  case KscCodePage:     return HANGUL_CHARSET;

  case 1254:            return TURKISH_CHARSET;
  case 1256:            return ARABIC_CHARSET;
  case 1258:            return VIETNAMESE_CHARSET;

  //default: return DEFAULT_CHARSET;
  default: return 0;
  }
}

// EOF
