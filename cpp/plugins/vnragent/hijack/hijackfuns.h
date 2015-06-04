#pragma once

// hijackfuns.h
// 6/3/2015 jichi

#include <windows.h>

namespace Hijack {

#define DEF_FUN(_fun, _return, ...) \
  typedef _return (WINAPI *_fun##_fun_t)(__VA_ARGS__); \
  extern _fun##_fun_t old##_fun; \
  _return WINAPI new##_fun(__VA_ARGS__);

  DEF_FUN(CreateFontIndirectA, HFONT, const LOGFONTA *lplf)
  DEF_FUN(CreateFontIndirectW, HFONT, const LOGFONTW *lplf)

  DEF_FUN(CreateFontA, HFONT, int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCSTR lpszFace)
  DEF_FUN(CreateFontW, HFONT, int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCWSTR lpszFace)

  DEF_FUN(GetGlyphOutlineA, DWORD, HDC hdc, UINT uChar, UINT uFormat, LPGLYPHMETRICS lpgm, DWORD cbBuffer, LPVOID lpvBuffer, const MAT2 *lpmat2)
  DEF_FUN(GetGlyphOutlineW, DWORD, HDC hdc, UINT uChar, UINT uFormat, LPGLYPHMETRICS lpgm, DWORD cbBuffer, LPVOID lpvBuffer, const MAT2 *lpmat2)

  DEF_FUN(GetTextExtentPoint32A, BOOL, HDC hdc, LPCSTR lpString, int cchString, LPSIZE lpSize)
  DEF_FUN(GetTextExtentPoint32W, BOOL, HDC hdc, LPCWSTR lpString, int cchString, LPSIZE lpSize)

#undef DEF_FUN

// Global variables

} // namespace Hijack

// EOF

/*

BOOL WINAPI newTextOutA(
  _In_  HDC hdc,
  _In_  int nXStart,
  _In_  int nYStart,
  _In_  LPCSTR lpString,
  _In_  int cchString
);

//BOOL WINAPI newTextOutW(
//  _In_  HDC hdc,
//  _In_  int nXStart,
//  _In_  int nYStart,
//  _In_  LPCWSTR lpString,
//  _In_  int cchString
//);

BOOL WINAPI newExtTextOutA(
  _In_  HDC hdc,
  _In_  int X,
  _In_  int Y,
  _In_  UINT fuOptions,
  _In_  const RECT *lprc,
  _In_  LPCSTR lpString,
  _In_  UINT cbCount,
  _In_  const INT *lpDx
);

//BOOL WINAPI newExtTextOutW(
//  _In_  HDC hdc,
//  _In_  int X,
//  _In_  int Y,
//  _In_  UINT fuOptions,
//  _In_  const RECT *lprc,
//  _In_  LPCWSTR lpString,
//  _In_  UINT cbCount,
//  _In_  const INT *lpDx
//);
*/
