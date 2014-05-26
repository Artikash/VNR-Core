// hijackfuncs_gdi32.cc
// 4/30/2014 jichi
#include "hijack/hijackfuncs_p.h"
#include "hijack/hijackhelper.h"
#include "util/codepage.h"

#define DEBUG "hijackfuncs_gdi32"
#include "sakurakit/skdebug.h"

//#define FONT_ZHS "楷体_GB2312"
//#define FONT_ZHS "KaiTi_GB2312"

HFONT WINAPI Hijack::myCreateFontIndirectA(const LOGFONTA *lplf)
{
  //DOUT("pass");
  HFONT ret = nullptr;
  if (auto p = HijackHelper::instance())
    if (auto charSet = p->systemCharSet())
      if (p->isTranscodingNeeded() && lplf) {
        LOGFONTA f(*lplf);
        f.lfCharSet = charSet;
        //:qstrcpy(f.lfFaceName, FONT_ZHS);
        //f.lfCharSet = GB2312_CHARSET;
        ret = ::CreateFontIndirectA(&f);
      }
  if (!ret)
    ret = ::CreateFontIndirectA(lplf);
  return ret;
}

HFONT WINAPI Hijack::myCreateFontIndirectW(const LOGFONTW *lplf)
{
  //DOUT("pass");
  HFONT ret = nullptr;
  if (auto p = HijackHelper::instance())
    if (auto charSet = p->systemCharSet())
      if (p->isTranscodingNeeded() && lplf) {
        LOGFONTW f(*lplf);
        f.lfCharSet = charSet;
        //:qstrcpy(f.lfFaceName, FONT_ZHS);
        //f.lfCharSet = GB2312_CHARSET;
        ret = ::CreateFontIndirectW(&f);
      }
  if (!ret)
    ret = ::CreateFontIndirectW(lplf);
  return ret;
}

HFONT WINAPI Hijack::myCreateFontA(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCSTR lpszFace)
{
  DOUT("pass");
  if (auto p = HijackHelper::instance())
    if (auto charSet = p->systemCharSet())
      if (p->isTranscodingNeeded()) {
        fdwCharSet = charSet;
        //:qstrcpy(lpszFace, FONT_ZHS);
      }
  return ::CreateFontA(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, lpszFace);
}

HFONT WINAPI Hijack::myCreateFontW(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCWSTR lpszFace)
{
  DOUT("pass");
  if (auto p = HijackHelper::instance())
    if (auto charSet = p->systemCharSet())
      if (p->isTranscodingNeeded()) {
        fdwCharSet = charSet;
        //:qstrcpy(lpszFace, FONT_ZHS);
      }
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

// TODO: Support extracting text from this function.
int WINAPI Engine::MyMultiByteToWideChar(
  _In_       UINT CodePage,
  _In_       DWORD dwFlags,
  _In_       LPCSTR lpMultiByteStr,
  _In_       int cbMultiByte,
  _Out_opt_  LPWSTR lpWideCharStr,
  _In_       int cchWideChar
)
{
  int ret = ::MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
  //if (ret > 1)
  //  qDebug() << QString::fromWCharArray(lpWideCharStr, ret);
  return ret;
}
*/
