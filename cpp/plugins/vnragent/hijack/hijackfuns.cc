// hijackfuns.cc
// 6/3/2015 jichi
#include "hijack/hijackfuns.h"
#include "hijack/hijackhelper.h"
#include "hijack/hijacksettings.h"
#include "util/dyncodec.h"
#include "util/textutil.h"

#define DEBUG "hijackfuns"
#include "sakurakit/skdebug.h"

// Disable only for debugging purpose
#define HIJACK_GDI_FONT
#define HIJACK_GDI_TEXT

#define DEF_FUN(_f) Hijack::_f##_fun_t Hijack::old##_f = ::_f;
  DEF_FUN(CreateFontA)
  DEF_FUN(CreateFontW)
  DEF_FUN(CreateFontIndirectA)
  DEF_FUN(CreateFontIndirectW)
  DEF_FUN(GetGlyphOutlineA)
  DEF_FUN(GetGlyphOutlineW)
  DEF_FUN(GetTextExtentPoint32A)
  DEF_FUN(GetTextExtentPoint32W)
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

class DCFontSwitcher
{
  HDC hdc_;
  HFONT oldFont_,
        newFont_;
public:
  explicit DCFontSwitcher(HDC hdc);
  ~DCFontSwitcher();
};

DCFontSwitcher::~DCFontSwitcher()
{
  if (oldFont_)
    ::SelectObject(hdc_, oldFont_);
  if (newFont_)
    ::DeleteObject(newFont_);
}

DCFontSwitcher::DCFontSwitcher(HDC hdc)
  : hdc_(hdc), oldFont_(nullptr), newFont_(nullptr)
{
  TEXTMETRIC tm;
  if (!::GetTextMetrics(hdc, &tm))
    return;

  auto p = HijackHelper::instance();
  if (!p)
    return;
  auto s = p->settings();
  if (!s->isFontCustomized())
    return;

  LOGFONTW lf = {};
  lf.lfHeight = tm.tmHeight;
  lf.lfWeight = tm.tmWeight;
  lf.lfItalic = tm.tmItalic;
  lf.lfUnderline = tm.tmUnderlined;
  lf.lfStrikeOut = tm.tmStruckOut;
  lf.lfCharSet = tm.tmCharSet;

  customizeLogFontW(&lf);

  newFont_ = Hijack::oldCreateFontIndirectW(&lf);
  oldFont_ = (HFONT)SelectObject(hdc_, newFont_);
  //DOUT("pass");
}

} // unnamed namespace

/** Fonts */

// http://forums.codeguru.com/showthread.php?500522-Need-clarification-about-CreateFontIndirect
// The font creation functions will never fail
HFONT WINAPI Hijack::newCreateFontIndirectA(const LOGFONTA *lplf)
{
#ifdef HIJACK_GDI_FONT
  //DOUT("pass");
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
#endif // HIJACK_GDI_FONT
  return oldCreateFontIndirectA(lplf);
}

HFONT WINAPI Hijack::newCreateFontIndirectW(const LOGFONTW *lplf)
{
#ifdef HIJACK_GDI_FONT
  //DOUT("pass");
  //DOUT("width:" << lplf->lfWidth << ", height:" << lplf->lfHeight << ", weight:" << lplf->lfWeight);
  if (auto p = HijackHelper::instance()) {
    auto s = p->settings();
    if (lplf && s->isFontCustomized()) {
      LOGFONTW lf(*lplf);
      customizeLogFontW(&lf);
      return oldCreateFontIndirectW(&lf);
    }
  }
#endif // HIJACK_GDI_FONT
  return oldCreateFontIndirectW(lplf);
}

HFONT WINAPI Hijack::newCreateFontA(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCSTR lpszFace)
{
#ifdef HIJACK_GDI_FONT
  //DOUT("pass");
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
#endif // HIJACK_GDI_FONT
  return oldCreateFontA(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, lpszFace);
}

HFONT WINAPI Hijack::newCreateFontW(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCWSTR lpszFace)
{
#ifdef HIJACK_GDI_FONT
  //DOUT("pass");
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
#endif // HIJACK_GDI_FONT
  return oldCreateFontW(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, lpszFace);
}

/** Text */

DWORD WINAPI Hijack::newGetGlyphOutlineA(HDC hdc, UINT uChar, UINT uFormat, LPGLYPHMETRICS lpgm, DWORD cbBuffer, LPVOID lpvBuffer, const MAT2 *lpmat2)
{
  //DCFontSwitcher fs(hdc);
#ifdef HIJACK_GDI_TEXT
  if (uChar > 0xff)
    if (auto p = DynamicCodec::instance()) {
      bool dynamic;
      UINT ch = p->decodeChar(uChar, &dynamic);
      if (dynamic && ch)
        return oldGetGlyphOutlineW(hdc, ch, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
    }
#endif // HIJACK_GDI_TEXT
  return oldGetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
}

BOOL WINAPI Hijack::newGetTextExtentPoint32A(HDC hdc, LPCSTR lpString, int cchString, LPSIZE lpSize)
{
  //DCFontSwitcher fs(hdc);
#ifdef HIJACK_GDI_TEXT
  //DOUT("pass");
  if(cchString > 1)
    if (auto p = DynamicCodec::instance()) {
      bool dynamic;
      QByteArray data(lpString, cchString);
      QString text = p->decode(data, &dynamic);
      if (dynamic && !text.isEmpty())
        return oldGetTextExtentPoint32W(hdc, (LPCWSTR)text.utf16(), text.size(), lpSize);
    }
#endif // HIJACK_GDI_TEXT
  return oldGetTextExtentPoint32A(hdc, lpString, cchString, lpSize);
}

// EOF

