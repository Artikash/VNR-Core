// hijackfuncs_gdi32.cc
// 4/30/2014 jichi
#include "hijack/hijackfuncs_p.h"
#include "hijack/hijackhelper.h"
#include "hijack/hijacksettings.h"
#include "util/codepage.h"

#define DEBUG "hijackfuncs_gdi32"
#include "sakurakit/skdebug.h"

//#define FONT_ZHS "楷体_GB2312"
//#define FONT_ZHS_A "KaiTi_GB2312"
//#define FONT_ZHS_W L"KaiTi_GB2312"

// Disable only for debugging purpose
#define HIJACK_GDI_FONT
//#define HIJACK_GDI_TEXT

// FIXME 6/16/2014: Why should I use 0x86 for charSet?
// See: http://i.watashi.me/archives/1.html
HFONT WINAPI Hijack::myCreateFontIndirectA(const LOGFONTA *lplf)
{
  HFONT ret = nullptr;
#ifdef HIJACK_GDI_FONT
  DOUT("pass");
  if (auto p = HijackHelper::instance()) {
    auto s = p->settings();
    if (lplf && (s->fontCharSetEnabled || !s->fontFamily.isEmpty())) {
      LOGFONTA f(*lplf);
      if (s->fontCharSetEnabled) {
        auto charSet = s->fontCharSet;
        if (!charSet)
          charSet = p->systemCharSet();
        if (charSet)
          f.lfCharSet = charSet;
      }
      if (!s->fontFamily.isEmpty())
        qstrcpy(f.lfFaceName, s->fontFamily.toLocal8Bit());
      ret = ::CreateFontIndirectA(&f);
    }
  }
#endif // HIJACK_GDI_FONT
  if (!ret)
    ret = ::CreateFontIndirectA(lplf);
  return ret;
}

HFONT WINAPI Hijack::myCreateFontIndirectW(const LOGFONTW *lplf)
{
  HFONT ret = nullptr;
#ifdef HIJACK_GDI_FONT
  DOUT("pass");
  if (auto p = HijackHelper::instance()) {
    auto s = p->settings();
    if (lplf && (s->fontCharSetEnabled || !s->fontFamily.isEmpty())) {
      LOGFONTW f(*lplf);
      if (s->fontCharSetEnabled) {
        auto charSet = s->fontCharSet;
        if (!charSet)
          charSet = p->systemCharSet();
        if (charSet)
          f.lfCharSet = charSet;
      }
      if (!s->fontFamily.isEmpty()) {
        f.lfFaceName[s->fontFamily.size()] = 0;
        s->fontFamily.toWCharArray(f.lfFaceName);
      }
      ret = ::CreateFontIndirectW(&f);
    }
  }
#endif // HIJACK_GDI_FONT
  if (!ret)
    ret = ::CreateFontIndirectW(lplf);
  return ret;
}

HFONT WINAPI Hijack::myCreateFontA(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCSTR lpszFace)
{
#ifdef HIJACK_GDI_FONT
  DOUT("pass");
  QByteArray ff;
  if (auto p = HijackHelper::instance()) {
    auto s = p->settings();
    if (s->fontCharSetEnabled || !s->fontFamily.isEmpty()) {
      if (s->fontCharSetEnabled) {
        auto charSet = s->fontCharSet;
        if (!charSet)
          charSet = p->systemCharSet();
        if (charSet)
          fdwCharSet = charSet;
      }
      if (!s->fontFamily.isEmpty()) {
        ff = s->fontFamily.toLocal8Bit();
        lpszFace = ff.constData();
      }
    }
  }
#endif // HIJACK_GDI_FONT
  return ::CreateFontA(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, lpszFace);
}

HFONT WINAPI Hijack::myCreateFontW(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCWSTR lpszFace)
{
#ifdef HIJACK_GDI_FONT
  DOUT("pass");
  if (auto p = HijackHelper::instance()) {
    auto s = p->settings();
    if (s->fontCharSetEnabled || !s->fontFamily.isEmpty()) {
      if (s->fontCharSetEnabled) {
        auto charSet = s->fontCharSet;
        if (!charSet)
          charSet = p->systemCharSet();
        if (charSet)
          fdwCharSet = charSet;
      }
      if (!s->fontFamily.isEmpty())
        lpszFace = (LPCWSTR)s->fontFamily.utf16();
    }
  }
#endif // HIJACK_GDI_FONT
  return ::CreateFontW(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, lpszFace);
}

// EOF

/*
#include <QtCore/QTextCodec>

BOOL WINAPI Hijack::myTextOutA(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, int cchString)
{
#ifdef HIJACK_GDI_TEXT
  DOUT("pass");
  QString t = Util::codecForName(ENC_SJIS)->toUnicode(lpString, cchString);
  if (!t.isEmpty())
    return ::TextOutW(hdc, nXStart, nYStart, (LPCWSTR)t.utf16(), t.size());
#endif // HIJACK_GDI_TEXT
  return ::TextOutA(hdc, nXStart, nYStart, lpString, cchString);
}

BOOL WINAPI Hijack::myExtTextOutA(HDC hdc, int X, int Y, UINT fuOptions, const RECT *lprc, LPCSTR lpString, UINT cbCount, const INT *lpDx)
{
#ifdef HIJACK_GDI_TEXT
  DOUT("pass");
  QString t = Util::codecForName(ENC_SJIS)->toUnicode(lpString, cbCount);
  if (!t.isEmpty())
    return ::ExtTextOutW(hdc, X, Y, fuOptions, lprc, (LPCWSTR)t.utf16(), t.size(), lpDx);
#endif // HIJACK_GDI_TEXT
  return ::ExtTextOutA(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
}

BOOL WINAPI Hijack::myGetTextExtentPoint32A(HDC hdc, LPCSTR lpString, int cchString, LPSIZE lpSize)
{
#ifdef HIJACK_GDI_TEXT
  DOUT("pass");
  QString t = Util::codecForName(ENC_SJIS)->toUnicode(lpString, cchString);
  if (!t.isEmpty())
    return ::GetTextExtentPoint32W(hdc, (LPCWSTR)t.utf16(), t.size(), lpSize);
#endif // HIJACK_GDI_TEXT
  return ::GetTextExtentPoint32A(hdc, lpString, cchString, lpSize);
}

DWORD WINAPI Hijack::myGetGlyphOutlineA(HDC hdc, UINT uChar, UINT uFormat, LPGLYPHMETRICS lpgm, DWORD cbBuffer, LPVOID lpvBuffer, const MAT2 *lpmat2)
{
#ifdef HIJACK_GDI_TEXT
  DOUT("pass");
#endif // HIJACK_GDI_TEXT
  return ::GetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
}

int WINAPI Hijack::myDrawTextA(HDC hDC, LPCSTR lpchText, int nCount, LPRECT lpRect, UINT uFormat)
{
#ifdef HIJACK_GDI_TEXT
  DOUT("pass");
#endif // HIJACK_GDI_TEXT
  return ::DrawTextA(hDC, lpchText, nCount, lpRect, uFormat);
}

int WINAPI Hijack::myDrawTextExA(HDC hdc, LPSTR lpchText, int cchText, LPRECT lprc, UINT dwDTFormat, LPDRAWTEXTPARAMS lpDTParams)
{
#ifdef HIJACK_GDI_TEXT
  DOUT("pass");
#endif // HIJACK_GDI_TEXT
  return ::DrawTextExA(hdc, lpchText, cchText, lprc, dwDTFormat, lpDTParams);
}

*/
