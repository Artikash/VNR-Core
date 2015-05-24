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

#define HIJACK_GDI32 // only for debugging purpose

// FIXME 6/16/2014: Why should I use 0x86 for charSet?
// See: http://i.watashi.me/archives/1.html
HFONT WINAPI Hijack::myCreateFontIndirectA(const LOGFONTA *lplf)
{
  //DOUT("pass");
  HFONT ret = nullptr;
#ifdef HIJACK_GDI32
  if (auto p = HijackHelper::instance()) {
    auto charSet = p->systemCharSet();
    if (lplf && (!charSet || charSet != lplf->lfCharSet) && p->isTranscodingNeeded()) {
      LOGFONTA f(*lplf);
      if (auto v = p->settings()->fontCharSet)
        charSet = v;
      if (charSet)
        f.lfCharSet = charSet;
      if (!p->settings()->fontFamily.isEmpty())
         qstrcpy(f.lfFaceName, p->settings()->fontFamily.toLocal8Bit());
      ret = ::CreateFontIndirectA(&f);
    }
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
  if (auto p = HijackHelper::instance()) {
    auto charSet = p->systemCharSet();
    if (lplf && (!charSet || charSet != lplf->lfCharSet) && p->isTranscodingNeeded()) {
      LOGFONTW f(*lplf);
      if (auto v = p->settings()->fontCharSet)
        charSet = v;
      if (charSet)
        f.lfCharSet = charSet;
      if (!p->settings()->fontFamily.isEmpty())
        p->settings()->fontFamily.toWCharArray(f.lfFaceName);
      ret = ::CreateFontIndirectW(&f);
    }
  }
#endif // HIJACK_GDI32
  if (!ret)
    ret = ::CreateFontIndirectW(lplf);
  return ret;
}

HFONT WINAPI Hijack::myCreateFontA(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCSTR lpszFace)
{
#ifdef HIJACK_GDI32
  //DOUT("pass");
  QByteArray ff;
  if (auto p = HijackHelper::instance()) {
    auto charSet = p->systemCharSet();
    if ((!charSet || charSet != fdwCharSet) && p->isTranscodingNeeded()) {
      if (auto v = p->settings()->fontCharSet)
        charSet = v;
      if (charSet)
        fdwCharSet = charSet;
      if (!p->settings()->fontFamily.isEmpty()) {
        ff = p->settings()->fontFamily.toLocal8Bit();
        lpszFace = ff.constData();
      }
    }
  }
#endif // HIJACK_GDI32
  return ::CreateFontA(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, lpszFace);
}

HFONT WINAPI Hijack::myCreateFontW(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCWSTR lpszFace)
{
#ifdef HIJACK_GDI32
  //DOUT("pass");
  if (auto p = HijackHelper::instance()) {
    auto charSet = p->systemCharSet();
    if ((!charSet || charSet != fdwCharSet) && p->isTranscodingNeeded()) {
      if (auto v = p->settings()->fontCharSet)
        charSet = v;
      if (charSet)
        fdwCharSet = charSet;
      if (!p->settings()->fontFamily.isEmpty())
        lpszFace = (LPCWSTR)p->settings()->fontFamily.utf16();
    }
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
  QString t = Util::codecForName("shift-jis")->toUnicode(lpString, cchString);
  if (!t.isEmpty())
    return ::TextOutW(hdc, nXStart, nYStart, (LPCWSTR)t.utf16(), t.size());
  else
    return ::TextOutA(hdc, nXStart, nYStart, lpString, cchString);
}
*/
