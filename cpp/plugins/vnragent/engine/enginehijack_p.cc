// eninehijack_p.cc
// 4/30/2014 jichi
#include "engine/enginehijack_p.h"
#include <QDebug>

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

BOOL WINAPI Engine::MyTextOutA(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, int cchString)
{
  qDebug() << QString::fromLocal8Bit(lpString, cchString);
  return ::TextOutA(hdc, nXStart, nYStart, lpString, cchString);
}

// EOF
