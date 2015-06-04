#pragma once

// hijackfuns.h
// 6/3/2015 jichi

#include <windows.h>

namespace Hijack {

// Function type definitions
typedef DWORD (WINAPI * GetGlyphOutlineA_fun_t)(HDC hdc, UINT uChar, UINT uFormat, LPGLYPHMETRICS lpgm, DWORD cbBuffer, LPVOID lpvBuffer, const MAT2 *lpmat2);
typedef BOOL (WINAPI * GetTextExtentPoint32A_fun_t)(HDC hdc, LPCSTR lpString, int cchString, LPSIZE lpSize);

// Global variables
extern GetGlyphOutlineA_fun_t oldGetGlyphOutlineA;
extern GetTextExtentPoint32A_fun_t oldGetTextExtentPoint32A;

// - GDI32 (text) -

DWORD WINAPI newGetGlyphOutlineA(
  _In_  HDC hdc,
  _In_  UINT uChar,
  _In_  UINT uFormat,
  _Out_ LPGLYPHMETRICS lpgm,
  _In_  DWORD cbBuffer,
  _Out_ LPVOID lpvBuffer,
  _In_  const MAT2 *lpmat2
);

//DWORD WINAPI newGetGlyphOutlineW(
//  _In_  HDC hdc,
//  _In_  UINT uChar,
//  _In_  UINT uFormat,
//  _Out_ LPGLYPHMETRICS lpgm,
//  _In_  DWORD cbBuffer,
//  _Out_ LPVOID lpvBuffer,
//  _In_  const MAT2 *lpmat2
//);

BOOL WINAPI newGetTextExtentPoint32A(
  _In_  HDC hdc,
  _In_  LPCSTR lpString,
  _In_  int cchString,
  _Out_ LPSIZE lpSize
);

//BOOL WINAPI newGetTextExtentPoint32W(
//  _In_  HDC hdc,
//  _In_  LPCWSTR lpString,
//  _In_  int cchString,
//  _Out_ LPSIZE lpSize
//);
//

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
