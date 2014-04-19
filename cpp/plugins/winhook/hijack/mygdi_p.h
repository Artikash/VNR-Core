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
  _In_  HDC hdc,
  _In_  int nXStart,
  _In_  int nYStart,
  _In_  LPCSTR lpString,
  _In_  int cchString
);
//BOOL WINAPI MyTextOutW(
//  _In_  HDC hdc,
//  _In_  int nXStart,
//  _In_  int nYStart,
//  _In_  LPCWSTR lpString,
//  _In_  int cchString
//);

#define MY_GDI_FUNCTIONS_INITIALIZER \
   { "TextOutA", "gdi32.dll", MyTextOutA }
// { "TextOutW", "gdi32.dll", MyTextOutW }

// EOF
