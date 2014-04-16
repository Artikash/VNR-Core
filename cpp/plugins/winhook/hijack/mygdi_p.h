#pragma once
// mygdi_p.h
// 4/14/2014 jichi
#include <windows.h>

struct MyFunctionInfo {
  LPCSTR functionName;
  LPCSTR moduleName;
  LPVOID functionAddress;
};

// - GDI32 -

BOOL WINAPI MyTextOutA(
  __in  HDC hdc,
  __in  int nXStart,
  __in  int nYStart,
  __in  LPCSTR lpString,
  __in  int cchString
);
//BOOL WINAPI MyTextOutW(
//  __in  HDC hdc,
//  __in  int nXStart,
//  __in  int nYStart,
//  __in  LPCWSTR lpString,
//  __in  int cchString
//);

#define MY_GDI_FUNCTIONS_INITIALIZER \
   { "TextOutA", "gdi32.dll", MyTextOutA }
// { "TextOutW", "gdi32.dll", MyTextOutW }

// EOF
