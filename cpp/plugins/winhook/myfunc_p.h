#pragma once
// myfunc_p.h
// 1/27/2013 jichi
#include <windows.h>

struct MyFunctionInfo {
  LPCSTR functionName;
  LPCSTR moduleName;
  LPVOID functionAddress;
};

// - KERNEL32 -

LPVOID WINAPI MyGetProcAddress(
  __in HMODULE hModule,
  __in LPCSTR lpProcName
);
HMODULE WINAPI MyLoadLibrary(
  __in  LPCTSTR lpFileName
);
HMODULE WINAPI MyLoadLibraryEx(
  __in LPCTSTR lpFileName,
  __reserved HANDLE hFile,
  __in DWORD dwFlags
);

// - USER32 -

BOOL WINAPI MyTrackPopupMenu(
  __in HMENU hMenu,
  __in UINT uFlags,
  __in int x,
  __in int y,
  __in int nReserved,
  __in HWND hWnd,
  __in_opt CONST RECT *prcRect
);
BOOL WINAPI MyTrackPopupMenuEx(
  __in HMENU hMenu,
  __in UINT uFlags,
  __in int x,
  __in int y,
  __in HWND hWnd,
  __in_opt LPTPMPARAMS lptpm
);

//int WINAPI MyMessageBoxA(
//  __in_opt HWND hWnd,
//  __in_opt LPCSTR lpText,
//  __in_opt LPCSTR lpCaption,
//  __in UINT uType
//);
//int WINAPI MyMessageBoxW(
//  __in HWND hWnd,
//  __in LPCWSTR lpText,
//  __in LPCWSTR lpCaption,
//  __in UINT uType
//);

// - GDI32 -

/* // CHECKPOINT
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
*/

// - DirectX -

//#include <d3d9.h>
//IDirect3D9* WINAPI MyDirect3DCreate9(
//  UINT SDKVersion
//);

#define MY_FUNCTIONS_INITIALIZER \
   { "GetProcAddress", "kernel32.dll", MyGetProcAddress }, \
   { "LoadLibrary", "kernel32.dll", MyLoadLibrary }, \
   { "LoadLibraryEx", "kernel32.dll", MyLoadLibraryEx }, \
   { "TrackPopupMenu", "user32.dll", MyTrackPopupMenu }, \
   { "TrackPopupMenuEx", "user32.dll", MyTrackPopupMenuEx }
//   { "TextOutA", "gdi32.dll", MyTextOutA }
//   { "GetDC", "gdi32.dll", MyGetDC }, \
//   { "GetDCEx", "gdi32.dll", MyGetDCEx }
// { "TextOutW", "gdi32.dll", MyTextOutW }
// { "MessageBoxA", "user32.dll", MyMessageBoxA }
// { "MessageBoxW", "user32.dll", MyMessageBoxW }
// { "Direct3DCreate9", "d3d9.dll", MyDirect3DCreate9 }

// EOF

/*
HDC WINAPI MyGetDC(
  __in HWND hWnd
);
HDC WINAPI MyGetDCEx(
  __in HWND hWnd,
  __in HRGN hrgnClip,
  __in DWORD flags
);

HDC WINAPI MyBeginPaint(
  __in HWND hWnd,
  __out LPPAINTSTRUCT lpPaint
);
#include <d3dx9core.h>
//
//INT WINAPI MyDrawTextW(
//  __in LPD3DXSPRITE pSprite,
//  __in LPCWSTR pString,
//  __in INT Count,
//  __in LPRECT pRect,
//  __in DWORD Format,
//  __in D3DCOLOR Color
//);

HRESULT MyD3DXCreateFontA(
  __in LPDIRECT3DDEVICE9 pDevice,
  __in INT Height,
  __in UINT Width,
  __in UINT Weight,
  __in UINT MipLevels,
  __in BOOL Italic,
  __in DWORD CharSet,
  __in DWORD OutputPrecision,
  __in DWORD Quality,
  __in DWORD PitchAndFamily,
  __in LPCSTR pFacename,
  __out LPD3DXFONT *ppFont
);

HRESULT MyD3DXCreateFontW(
  __in LPDIRECT3DDEVICE9 pDevice,
  __in INT Height,
  __in UINT Width,
  __in UINT Weight,
  __in UINT MipLevels,
  __in BOOL Italic,
  __in DWORD CharSet,
  __in DWORD OutputPrecision,
  __in DWORD Quality,
  __in DWORD PitchAndFamily,
  __in LPCWSTR pFacename,
  __out LPD3DXFONT *ppFont
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
