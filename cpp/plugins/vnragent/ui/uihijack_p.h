#pragma once

// uihijack_p.h
// 1/27/2013 jichi

#include <windows.h>

#define UI_HIJACK_FUNCTIONS_INITIALIZER \
   { "GetProcAddress", "kernel32.dll", Ui::MyGetProcAddress }, \
   { "LoadLibrary", "kernel32.dll", Ui::MyLoadLibrary }, \
   { "LoadLibraryEx", "kernel32.dll", Ui::MyLoadLibraryEx }, \
   { "TrackPopupMenu", "user32.dll", Ui::MyTrackPopupMenu }, \
   { "TrackPopupMenuEx", "user32.dll", Ui::MyTrackPopupMenuEx }

namespace Ui {

struct FunctionInfo {
  LPCSTR functionName;
  LPCSTR moduleName;
  LPVOID functionAddress;
};

void overrideModuleFunctions(HMODULE hModule);

// - KERNEL32 -

LPVOID WINAPI MyGetProcAddress(
  _In_ HMODULE hModule,
  _In_ LPCSTR lpProcName
);
HMODULE WINAPI MyLoadLibrary(
  _In_  LPCTSTR lpFileName
);
HMODULE WINAPI MyLoadLibraryEx(
  _In_ LPCTSTR lpFileName,
  __reserved HANDLE hFile,
  _In_ DWORD dwFlags
);

// - USER32 -

BOOL WINAPI MyTrackPopupMenu(
  _In_ HMENU hMenu,
  _In_ UINT uFlags,
  _In_ int x,
  _In_ int y,
  _In_ int nReserved,
  _In_ HWND hWnd,
  _In_opt_ CONST RECT *prcRect
);
BOOL WINAPI MyTrackPopupMenuEx(
  _In_ HMENU hMenu,
  _In_ UINT uFlags,
  _In_ int x,
  _In_ int y,
  _In_ HWND hWnd,
  _In_opt_ LPTPMPARAMS lptpm
);

} // namespace Ui

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

/* // CHECKPOINT
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

// - DirectX -

//#include <d3d9.h>
//IDirect3D9* WINAPI MyDirect3DCreate9(
//  UINT SDKVersion
//);

/*
HDC WINAPI MyGetDC(
  _In_ HWND hWnd
);
HDC WINAPI MyGetDCEx(
  _In_ HWND hWnd,
  _In_ HRGN hrgnClip,
  _In_ DWORD flags
);

HDC WINAPI MyBeginPaint(
  _In_ HWND hWnd,
  _Out_ LPPAINTSTRUCT lpPaint
);
#include <d3dx9core.h>
//
//INT WINAPI MyDrawTextW(
//  _In_ LPD3DXSPRITE pSprite,
//  _In_ LPCWSTR pString,
//  _In_ INT Count,
//  _In_ LPRECT pRect,
//  _In_ DWORD Format,
//  _In_ D3DCOLOR Color
//);

HRESULT MyD3DXCreateFontA(
  _In_ LPDIRECT3DDEVICE9 pDevice,
  _In_ INT Height,
  _In_ UINT Width,
  _In_ UINT Weight,
  _In_ UINT MipLevels,
  _In_ BOOL Italic,
  _In_ DWORD CharSet,
  _In_ DWORD OutputPrecision,
  _In_ DWORD Quality,
  _In_ DWORD PitchAndFamily,
  _In_ LPCSTR pFacename,
  _Out_ LPD3DXFONT *ppFont
);

HRESULT MyD3DXCreateFontW(
  _In_ LPDIRECT3DDEVICE9 pDevice,
  _In_ INT Height,
  _In_ UINT Width,
  _In_ UINT Weight,
  _In_ UINT MipLevels,
  _In_ BOOL Italic,
  _In_ DWORD CharSet,
  _In_ DWORD OutputPrecision,
  _In_ DWORD Quality,
  _In_ DWORD PitchAndFamily,
  _In_ LPCWSTR pFacename,
  _Out_ LPD3DXFONT *ppFont
);
   { "D3DXCreateFontA", "d3dx9_24.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_24.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9_25.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_25.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9_26.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_26.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9_27.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_27.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9_28.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_28.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9_29.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_29.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9_30.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_30.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9_31.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_31.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9_32.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_32.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9_33.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_33.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9_34.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_34.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9_35.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_35.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9_36.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_36.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9_37.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_37.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9_38.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_38.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9_39.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_39.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9_40.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_40.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9_41.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_41.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9_42.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_42.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9_43.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9_43.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9d_33.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9d_33.dll", MyD3DXCreateFontW },
   { "D3DXCreateFontA", "d3dx9d_43.dll", MyD3DXCreateFontA },
   { "D3DXCreateFontW", "d3dx9d_43.dll", MyD3DXCreateFontW }
*/
