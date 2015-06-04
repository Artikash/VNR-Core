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
      } f = {*lplf}; // only initialize the first member of LOGFONTA
      if (s->fontCharSetEnabled) {
        auto charSet = s->fontCharSet;
        if (!charSet)
          charSet = p->systemCharSet();
        if (charSet)
          f.a.lfCharSet = charSet;
      }
      if (s->fontWeight)
        f.a.lfWeight = s->fontWeight;
      if (s->isFontScaled()) {
        f.a.lfWidth *= s->fontScale;
        f.a.lfHeight *= s->fontScale;
      }
      if (!s->fontFamily.isEmpty()) {
        if (Util::allAscii(s->fontFamily))
          ::strcpy(f.a.lfFaceName, s->fontFamily.toLocal8Bit());
        else {
          f.w.lfFaceName[s->fontFamily.size()] = 0;
          s->fontFamily.toWCharArray(f.w.lfFaceName);
          return oldCreateFontIndirectW(&f.w);
        }
      }
      return oldCreateFontIndirectA(&f.a);
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
      LOGFONTW f(*lplf);
      if (s->fontCharSetEnabled) {
        auto charSet = s->fontCharSet;
        if (!charSet)
          charSet = p->systemCharSet();
        if (charSet)
          f.lfCharSet = charSet;
      }
      if (s->fontWeight)
        f.lfWeight = s->fontWeight;
      if (s->isFontScaled()) {
        f.lfWidth *= s->fontScale;
        f.lfHeight *= s->fontScale;
      }
      if (!s->fontFamily.isEmpty()) {
        f.lfFaceName[s->fontFamily.size()] = 0;
        s->fontFamily.toWCharArray(f.lfFaceName);
      }
      return oldCreateFontIndirectW(&f);
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

