// hijackfuncs_engine.cc
// 4/30/2014 jichi
#include "hijack/hijackfuncs_p.h"
#include "util/codepage.h"
#include "engine/engine.h"
//#include <QtCore/QString>
//#include <QtCore/QTextCodec>

//#define FONT_ZHS "楷体_GB2312"
//#define FONT_ZHS "KaiTi_GB2312"

namespace { // unnamed


} // namespace

// This only works before starting the game
HFONT WINAPI Hijack::MyCreateFontIndirectA(const LOGFONTA *lplf)
{
  HFONT ret = nullptr;
  if (auto charSet = Util::currentCharSet())
    if (auto p = AbstractEngine::instance())
      if (p->isTranscodingNeeded() && lplf) {
        LOGFONTA lf(*lplf);
        //::strcpy(lf.lfFaceName, FONT_ZHS);
        //lf.lfCharSet = GB2312_CHARSET;
        lf.lfCharSet = charSet;
        ret = ::CreateFontIndirectA(&lf);
      }
  if (!ret)
    ret = ::CreateFontIndirectA(lplf);
  return ret;
}

// EOF

/*
BOOL WINAPI Engine::MyTextOutA(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, int cchString)
{
  //qDebug() << QString::fromLocal8Bit(lpString, cchString);
  //return ::TextOutA(hdc, nXStart, nYStart, lpString, cchString);
  QString t = QTextCodec::codecForName("GB2312")->toUnicode(lpString, cchString);
  if (!t.isEmpty())
    return ::TextOutW(hdc, nXStart, nYStart, t.toStdWString().c_str(), t.size());
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
