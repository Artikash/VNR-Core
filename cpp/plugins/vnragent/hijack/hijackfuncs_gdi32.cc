// hijackfuncs_gdi32.cc
// 4/30/2014 jichi
#include "hijack/hijackfuncs_p.h"
#include "hijack/hijackhelper.h"
#include "util/codepage.h"

#define DEBUG "hijackfuncs_gdi32"
#include "sakurakit/skdebug.h"

//#define FONT_ZHS "楷体_GB2312"
#define FONT_ZHS_A "KaiTi_GB2312"
#define FONT_ZHS_W L"KaiTi_GB2312"

#define HIJACK_GDI32 // only for debugging purpose

// FIXME 6/16/2014: Why should I use 0x86 for charSet?
// See: http://i.watashi.me/archives/1.html
HFONT WINAPI Hijack::myCreateFontIndirectA(const LOGFONTA *lplf)
{
  //DOUT("pass");
  HFONT ret = nullptr;
#ifdef HIJACK_GDI32
  if (auto p = HijackHelper::instance())
    if (auto charSet = p->systemCharSet())
      if (p->isTranscodingNeeded() && lplf) {
        LOGFONTA f(*lplf);
        f.lfCharSet = charSet;
        //::strcpy(f.lfFaceName, FONT_ZHS_A);
        //f.lfCharSet = GB2312_CHARSET;
        ret = ::CreateFontIndirectA(&f);
      }
#endif // HIJACK_GDI32
  if (!ret)
    ret = ::CreateFontIndirectA(lplf);
  return ret;
}

HFONT WINAPI Hijack::myCreateFontIndirectW(const LOGFONTW *lplf)
{
  //DOUT("pass");
  HFONT ret = nullptr;
#ifdef HIJACK_GDI32
  if (auto p = HijackHelper::instance())
    if (auto charSet = p->systemCharSet())
      if (p->isTranscodingNeeded() && lplf) {
        LOGFONTW f(*lplf);
        f.lfCharSet = charSet;
        //::wcscpy(f.lfFaceName, FONT_ZHS_W);
        ret = ::CreateFontIndirectW(&f);
      }
#endif // HIJACK_GDI32
  if (!ret)
    ret = ::CreateFontIndirectW(lplf);
  return ret;
}

HFONT WINAPI Hijack::myCreateFontA(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCSTR lpszFace)
{
#ifdef HIJACK_GDI32
  DOUT("pass");
  if (auto p = HijackHelper::instance())
    if (auto charSet = p->systemCharSet())
      if (p->isTranscodingNeeded()) {
        fdwCharSet = charSet;

        //static const char face[] = FONT_ZHS_A;
        //lpszFace = face;
      }
#endif // HIJACK_GDI32
  return ::CreateFontA(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, lpszFace);
}

HFONT WINAPI Hijack::myCreateFontW(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCWSTR lpszFace)
{
#ifdef HIJACK_GDI32
  DOUT("pass");
  if (auto p = HijackHelper::instance())
    if (auto charSet = p->systemCharSet())
      if (p->isTranscodingNeeded()) {
        fdwCharSet = charSet;

        //static const wchar_t face[] = FONT_ZHS_W;
        //lpszFace = face;
      }
#endif // HIJACK_GDI32
  return ::CreateFontW(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, lpszFace);
}

// EOF

/*
#include <QTextCodec>
BOOL WINAPI Hijack::myTextOutA(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, int cchString)
{
  //qDebug() << QString::fromLocal8Bit(lpString, cchString);
  //return ::TextOutA(hdc, nXStart, nYStart, lpString, cchString);
  QString t = QTextCodec::codecForName("shift-jis")->toUnicode(lpString, cchString);
  if (!t.isEmpty())
    return ::TextOutW(hdc, nXStart, nYStart, (LPCWSTR)t.utf16(), t.size());
  else
    return ::TextOutA(hdc, nXStart, nYStart, lpString, cchString);
}
*/
