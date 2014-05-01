#pragma once

// eninehijack_p.h
// 1/27/2013 jichi

#include <windows.h>

#define ENGINE_HIJACK_FUNCTIONS_INITIALIZER \
   { "MultiByteToWideChar", "kernel32.dll", Engine::MyMultiByteToWideChar }

namespace Engine {

struct FunctionInfo {
  LPCSTR functionName;
  LPCSTR moduleName;
  LPVOID functionAddress;
};

void overrideModuleFunctions(HMODULE hModule);

// - KERNEL32 -

int WINAPI MyMultiByteToWideChar(
  _In_       UINT CodePage,
  _In_       DWORD dwFlags,
  _In_       LPCSTR lpMultiByteStr,
  _In_       int cbMultiByte,
  _Out_opt_  LPWSTR lpWideCharStr,
  _In_       int cchWideChar
);

} // namespace Engine

// EOF

//   { "TextOutA", "gdi32.dll", MyTextOutA }
//   { "GetDC", "gdi32.dll", MyGetDC }, \
//   { "GetDCEx", "gdi32.dll", MyGetDCEx }
// { "TextOutW", "gdi32.dll", MyTextOutW }
// { "MessageBoxA", "user32.dll", MyMessageBoxA }
// { "MessageBoxW", "user32.dll", MyMessageBoxW }
// { "Direct3DCreate9", "d3d9.dll", MyDirect3DCreate9 }

//int WINAPI MyMessageBoxA(
//  _In_opt_ HWND hWnd,
//  _In_opt_ LPCSTR lpText,
//  _In_opt_ LPCSTR lpCaption,
//  _In_ UINT uType
//);
//int WINAPI MyMessageBoxW(
//  _In_ HWND hWnd,
//  _In_ LPCWSTR lpText,
//  _In_ LPCWSTR lpCaption,
//  _In_ UINT uType
//);

/*
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
*/
