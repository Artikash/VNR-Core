// hijackfuns.cc
// 6/3/2015 jichi
#include "hijack/hijackfuns.h"
#include "hijack/hijackhelper.h"
#include "hijack/hijacksettings.h"
#include "util/dyncodec.h"
#include "util/textutil.h"
#include "dyncodec/dynsjis.h"
#include <boost/foreach.hpp>
#include <algorithm>
#include <list>
#include <utility>

#define DEBUG "hijackfuns"
#include "sakurakit/skdebug.h"

// Disable only for debugging purpose
//#define HIJACK_GDI_FONT
//#define HIJACK_GDI_TEXT

#define DEF_FUN(_f) Hijack::_f##_fun_t Hijack::old##_f = ::_f;
  DEF_FUN(CreateFontA)
  DEF_FUN(CreateFontW)
  DEF_FUN(CreateFontIndirectA)
  DEF_FUN(CreateFontIndirectW)
  DEF_FUN(GetGlyphOutlineA)
  DEF_FUN(GetGlyphOutlineW)
  DEF_FUN(GetTextExtentPoint32A)
  DEF_FUN(GetTextExtentPoint32W)
  DEF_FUN(GetTextExtentExPointA)
  DEF_FUN(GetTextExtentExPointW)
  DEF_FUN(GetCharABCWidthsA)
  DEF_FUN(GetCharABCWidthsW)
  DEF_FUN(TextOutA)
  DEF_FUN(TextOutW)
  DEF_FUN(ExtTextOutA)
  DEF_FUN(ExtTextOutW)
  //DEF_FUN(TabbedTextOutA)
  //DEF_FUN(TabbedTextOutW)
  DEF_FUN(CharNextA)
  //DEF_FUN(CharNextW)
  //DEF_FUN(CharNextExA)
  //DEF_FUN(CharNextExW)
  DEF_FUN(MultiByteToWideChar)
  DEF_FUN(WideCharToMultiByte)
#undef DEF_FUN

/** Helper */

namespace { // unnamed

void customizeLogFontA(LOGFONTA *lplf)
{
  auto p = HijackHelper::instance();
  auto s = p->settings();
  if (s->fontCharSetEnabled) {
    auto charSet = s->fontCharSet;
    if (!charSet)
      charSet = p->systemCharSet();
    if (charSet)
      lplf->lfCharSet = charSet;
  }
  if (s->fontWeight)
    lplf->lfWeight = s->fontWeight;
  if (s->isFontScaled()) {
    lplf->lfWidth *= s->fontScale;
    lplf->lfHeight *= s->fontScale;
  }
}

void customizeLogFontW(LOGFONTW *lplf)
{
  customizeLogFontA((LOGFONTA *)lplf);

  auto p = HijackHelper::instance();
  auto s = p->settings();
  if (!s->fontFamily.isEmpty()) {
    lplf->lfFaceName[s->fontFamily.size()] = 0;
    s->fontFamily.toWCharArray(lplf->lfFaceName);
  }
}

// LogFont manager

class LogFontManager
{
  typedef std::pair<HFONT, LOGFONTW> font_pair;
  std::list<font_pair> fonts_;

  static bool eq(const LOGFONTW &x, const LOGFONTW&y);

public:
  HFONT get(const LOGFONTW &lf) const;
  void add(HFONT hf, const LOGFONTW &lf);
  void remove(HFONT hf);
  void remove(const LOGFONTW &lf);
};

bool LogFontManager::eq(const LOGFONTW &x, const LOGFONTW &y)
{ // I assume there is no padding
  return ::wcscmp(x.lfFaceName, y.lfFaceName) == 0
      && ::memcmp(&x, &y, sizeof(x) - sizeof(x.lfFaceName)) == 0;
}

void LogFontManager::add(HFONT hf, const LOGFONTW &lf)
{ fonts_.push_back(std::make_pair(hf, lf)); }

void LogFontManager::remove(HFONT hf)
{
  std::remove_if(fonts_.begin(), fonts_.end(), [&hf](const font_pair &it) {
    return it.first == hf;
  });
}

void LogFontManager::remove(const LOGFONTW &lf)
{
  std::remove_if(fonts_.begin(), fonts_.end(), [&lf](const font_pair &it) {
    return eq(it.second, lf);
  });
}

HFONT LogFontManager::get(const LOGFONTW &lf) const
{
  BOOST_FOREACH (const font_pair &it, fonts_)
    if (eq(it.second, lf))
      return it.first;
  return nullptr;
}

// GDI font switcher

class DCFontSwitcher
{
  static LogFontManager fonts_;

  HDC hdc_;
  HFONT oldFont_,
        newFont_;

public:
  explicit DCFontSwitcher(HDC hdc); // pass 0 to disable this class
  ~DCFontSwitcher();
};

LogFontManager DCFontSwitcher::fonts_;

DCFontSwitcher::~DCFontSwitcher()
{
  // No idea why selecting old font will crash Mogeko Castle
  //if (oldFont_ && oldFont_ != HGDI_ERROR)
  //  ::SelectObject(hdc_, oldFont_);

  // Never delete new font but cache them
  // This could result in bad font after game is reset and deleted my font
  //if (newFont_)
  //  ::DeleteObject(newFont_);
}

DCFontSwitcher::DCFontSwitcher(HDC hdc)
  : hdc_(hdc), oldFont_(nullptr), newFont_(nullptr)
{
  if (!hdc_)
    return;
  auto p = HijackHelper::instance();
  if (!p)
    return;
  auto s = p->settings();
  if (!s->deviceContextFontEnabled || !s->isFontCustomized())
    return;

  TEXTMETRICW tm;
  if (!::GetTextMetricsW(hdc, &tm))
    return;

  LOGFONTW lf = {};
  lf.lfHeight = tm.tmHeight;
  lf.lfWeight = tm.tmWeight;
  lf.lfItalic = tm.tmItalic;
  lf.lfUnderline = tm.tmUnderlined;
  lf.lfStrikeOut = tm.tmStruckOut;
  lf.lfCharSet = tm.tmCharSet;
  lf.lfPitchAndFamily = tm.tmPitchAndFamily;

  customizeLogFontW(&lf);

  if (s->fontFamily.isEmpty())
    ::GetTextFaceW(hdc_, LF_FACESIZE, lf.lfFaceName);

  newFont_ = fonts_.get(lf);
  if (!newFont_) {
    newFont_ = Hijack::oldCreateFontIndirectW(&lf);
    fonts_.add(newFont_, lf);
  }
  oldFont_ = (HFONT)SelectObject(hdc_, newFont_);
  DOUT("pass");
}

} // unnamed namespace

/** Fonts */

// http://forums.codeguru.com/showthread.php?500522-Need-clarification-about-CreateFontIndirect
// The font creation functions will never fail
HFONT WINAPI Hijack::newCreateFontIndirectA(const LOGFONTA *lplf)
{
  DOUT("pass");
  //DOUT("width:" << lplf->lfWidth << ", height:" << lplf->lfHeight << ", weight:" << lplf->lfWeight);
  if (auto p = HijackHelper::instance()) {
    auto s = p->settings();
    if (lplf && s->isFontCustomized()) {
      union {
        LOGFONTA a;
        LOGFONTW w;
      } lf = {*lplf}; // only initialize the first member of LOGFONTA
      customizeLogFontA(&lf.a);
      if (!s->fontFamily.isEmpty()) {
        if (Util::allAscii(s->fontFamily))
          ::strcpy(lf.a.lfFaceName, s->fontFamily.toLocal8Bit());
        else {
          lf.w.lfFaceName[s->fontFamily.size()] = 0;
          s->fontFamily.toWCharArray(lf.w.lfFaceName);
          return oldCreateFontIndirectW(&lf.w);
        }
      }
      return oldCreateFontIndirectA(&lf.a);
    }
  }
  return oldCreateFontIndirectA(lplf);
}

HFONT WINAPI Hijack::newCreateFontIndirectW(const LOGFONTW *lplf)
{
  DOUT("pass");
  //DOUT("width:" << lplf->lfWidth << ", height:" << lplf->lfHeight << ", weight:" << lplf->lfWeight);
  if (auto p = HijackHelper::instance()) {
    auto s = p->settings();
    if (lplf && s->isFontCustomized()) {
      LOGFONTW lf(*lplf);
      customizeLogFontW(&lf);
      return oldCreateFontIndirectW(&lf);
    }
  }
  return oldCreateFontIndirectW(lplf);
}

HFONT WINAPI Hijack::newCreateFontA(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCSTR lpszFace)
{
  DOUT("pass");
  if (auto p = HijackHelper::instance()) {
    auto s = p->settings();
    if (s->isFontCustomized()) {
      if (s->fontCharSetEnabled) {
        auto charSet = s->fontCharSet;
        if (!charSet)
          charSet = p->systemCharSet();
        if (charSet)
          fdwCharSet = charSet;
      }
      if (s->fontWeight)
        fnWeight = s->fontWeight;
      if (s->isFontScaled()) {
        nWidth *= s->fontScale;
        nHeight *= s->fontScale;
      }
      if (!s->fontFamily.isEmpty()) {
        if (Util::allAscii(s->fontFamily)) {
          QByteArray ff = s->fontFamily.toLocal8Bit();
          lpszFace = ff.constData();
          return oldCreateFontA(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, lpszFace);
        } else {
          auto lpszFace = (LPCWSTR)s->fontFamily.utf16();
          return oldCreateFontW(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, lpszFace);
        }
      }
    }
  }
  return oldCreateFontA(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, lpszFace);
}

HFONT WINAPI Hijack::newCreateFontW(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCWSTR lpszFace)
{
  DOUT("pass");
  if (auto p = HijackHelper::instance()) {
    auto s = p->settings();
    if (s->isFontCustomized()) {
      if (s->fontCharSetEnabled) {
        auto charSet = s->fontCharSet;
        if (!charSet)
          charSet = p->systemCharSet();
        if (charSet)
          fdwCharSet = charSet;
      }
      if (s->fontWeight)
        fnWeight = s->fontWeight;
      if (s->isFontScaled()) {
        nWidth *= s->fontScale;
        nHeight *= s->fontScale;
      }
      if (!s->fontFamily.isEmpty())
        lpszFace = (LPCWSTR)s->fontFamily.utf16();
    }
  }
  return oldCreateFontW(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, lpszFace);
}

/** Encoding */

int WINAPI Hijack::newMultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
{
  DOUT("pass");
  if (auto p = DynamicCodec::instance())
    if ((CodePage == 0 || CodePage == 932) && cchWideChar > 0 && cbMultiByte > 1) {
      bool dynamic;
      QByteArray data(lpMultiByteStr, cbMultiByte);
      QString text = p->decode(data, &dynamic);
      if (dynamic && !text.isEmpty()) {
        int size = min(text.size() + 1, cchWideChar);
        ::memcpy(lpWideCharStr, text.utf16(), size * 2);
        //lpWideCharStr[size - 1] = 0; // enforce trailing zero
        return size - 1;
      }
    }
  return oldMultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}

int WINAPI Hijack::newWideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar)
{
  DOUT("pass");
  if (auto p = DynamicCodec::instance())
    if ((CodePage == 0 || CodePage == 932) && cchWideChar > 0 && cbMultiByte >= 0) {
      bool dynamic;
      QString text = QString::fromWCharArray(lpWideCharStr, cchWideChar);
      QByteArray data = p->encode(text, &dynamic);
      if (dynamic && !data.isEmpty()) {
        int size = data.size() + 1;
        if (cbMultiByte && cbMultiByte < size)
          size = cbMultiByte;
        ::memcpy(lpMultiByteStr, data.constData(), size);
        //lpMultiByteStr[size - 1] = 0; // enforce trailing zero
        return size - 1;
      }
    }
  return oldWideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
}

/** Text */

#define DECODE_TEXT(lpString, cchString, ...) \
{ \
  if(cchString > 1) \
    if (auto p = DynamicCodec::instance()) { \
      bool dynamic; \
      QByteArray data(lpString, cchString); \
      QString text = p->decode(data, &dynamic); \
      if (dynamic && !text.isEmpty()) { \
        LPCWSTR lpString = (LPCWSTR)text.utf16(); \
        cchString = text.size(); \
        return (__VA_ARGS__); \
      } \
    } \
}

#define DECODE_CHAR(uChar, ...) \
{ \
  if (uChar > 0xff) \
    if (auto p = DynamicCodec::instance()) { \
      bool dynamic; \
      UINT ch = p->decodeChar(uChar, &dynamic); \
      if (dynamic && ch) { \
        uChar = ch; \
        return (__VA_ARGS__); \
      } \
    } \
}

DWORD WINAPI Hijack::newGetGlyphOutlineA(HDC hdc, UINT uChar, UINT uFormat, LPGLYPHMETRICS lpgm, DWORD cbBuffer, LPVOID lpvBuffer, const MAT2 *lpmat2)
{
  DOUT("pass");
  DCFontSwitcher fs(hdc);
  DECODE_CHAR(uChar, oldGetGlyphOutlineW(hdc, ch, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2))
  return oldGetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
}

DWORD WINAPI Hijack::newGetGlyphOutlineW(HDC hdc, UINT uChar, UINT uFormat, LPGLYPHMETRICS lpgm, DWORD cbBuffer, LPVOID lpvBuffer, const MAT2 *lpmat2)
{
  DOUT("pass");
  DCFontSwitcher fs(hdc);
  return oldGetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
}

BOOL WINAPI Hijack::newGetTextExtentPoint32A(HDC hdc, LPCSTR lpString, int cchString, LPSIZE lpSize)
{
  DOUT("pass");
  DCFontSwitcher fs(hdc);
  DECODE_TEXT(lpString, cchString, oldGetTextExtentPoint32W(hdc, lpString, cchString, lpSize))
  return oldGetTextExtentPoint32A(hdc, lpString, cchString, lpSize);
}

BOOL WINAPI Hijack::newGetTextExtentPoint32W(HDC hdc, LPCWSTR lpString, int cchString, LPSIZE lpSize)
{
  DOUT("pass");
  DCFontSwitcher fs(hdc);
  return oldGetTextExtentPoint32W(hdc, lpString, cchString, lpSize);
}

BOOL WINAPI Hijack::newTextOutA(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, int cchString)
{
  DOUT("pass");
  DCFontSwitcher fs(hdc);
  DECODE_TEXT(lpString, cchString, oldTextOutW(hdc, nXStart, nYStart, lpString, cchString))
  return oldTextOutA(hdc, nXStart, nYStart, lpString, cchString);
}

BOOL WINAPI Hijack::newTextOutW(HDC hdc, int nXStart, int nYStart, LPCWSTR lpString, int cchString)
{
  DOUT("pass");
  DCFontSwitcher fs(hdc);
  return oldTextOutW(hdc, nXStart, nYStart, lpString, cchString);
}

LPSTR WINAPI Hijack::newCharNextA(LPCSTR lpString)
{
  DOUT("pass");
  //if (::GetACP() == 932)
  return const_cast<char *>(dynsjis::next_char(lpString));
  //return oldCharNextA(lpString);
}

// EOF
