// mygdi.cc
// 4/14/2014 jichi
#include "winhook/hijack/mygdi.h"
#include "winhook/hijack/mygdi_p.h"
//#include "winhook/qt/mainobj.h"
#include "winhook/util/winsec.h"
#include "winhook/myfunc.h"
#include "winhook/myfunc_p.h"
#include <psapi.h>
#include <boost/foreach.hpp>

#ifdef _MSC_VER
# pragma warning (disable:4996)   // C4996: use POSIX function (wcscat)
#endif // _MSC_VER

#define DEBUG "mygdi"
#ifdef DEBUG
# include "growl.h"
#endif // DEBUG

// - Helpers -

namespace { // unnamed

const MyFunctionInfo MY_FUNCTIONS[] = { MY_GDI_FUNCTIONS_INITIALIZER };

enum { PATH_SEP = '\\' };

inline const char *basename(const char *s)
{
  if (const char *ret = ::strrchr(s, PATH_SEP))
    return ++ret;
  else
    return s;
}

inline const wchar_t *basename(const wchar_t *s)
{
  if (const wchar_t *ret = ::wcsrchr(s, PATH_SEP))
    return ++ret; // skip the path seperator
  else
    return s;
}

inline LPCWSTR applicationPathW()
{
  static WCHAR ret[MAX_PATH];
  if (!*ret)
    ::GetModuleFileNameW(nullptr, ret, MAX_PATH);
  return ret;
}

inline LPCWSTR applicationNameW()
{
  static LPCWSTR ret = nullptr;
  if (!ret && (ret = wcsrchr(applicationPathW(), PATH_SEP)))
    ret++;  // skip the path seperator
  return ret;
}

} // unnamed namespace

// - Hooker -

//BOOL (WINAPI *OldTextOutA)(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, int cchString) = TextOutA;
void My::OverrideGDIModuleFunctions(HMODULE hModule)
{
 // growl::debug("override GDI functions");
  BOOST_FOREACH (const MyFunctionInfo &fn, MY_FUNCTIONS) {
#ifdef DEBUG
    PVOID ret = winsec::OverrideFunctionA(hModule, fn.moduleName, fn.functionName, fn.functionAddress);
    if (ret)
      growl::debug(fn.functionName); // success
#else
    winsec::OverrideFunctionA(hModule, fn.moduleName, fn.functionName, fn.functionAddress);
#endif // DEBUG
  }
}

void My::OverrideGDIModules()
{
  LPCWSTR exeName = applicationNameW();
  if (!exeName)
    return;
  LPCWSTR exePath = applicationPathW();

  // For each matched module, override functions
  enum { MAX_MODULE = 0x800 };
  WCHAR path[MAX_PATH];
  HMODULE modules[MAX_MODULE];
  DWORD size;
  if (::EnumProcessModules(::GetCurrentProcess(), modules, sizeof(modules), &size) && (size/=4))
    for (size_t i = 0; i < size; i++)
      if (::GetModuleFileNameW(modules[i], path, sizeof(path)/sizeof(*path)) &&
          !::wcsnicmp(path, exePath, exeName - exePath))
        OverrideGDIModuleFunctions(modules[i]);
}

// - My Functions -


BOOL hijack_gdi = FALSE;

// CHECKPOINT: How to handle memory DC?
BOOL WINAPI MyTextOutA(
  _In_  HDC hdc,
  _In_  int nXStart,
  _In_  int nYStart,
  _In_  LPCSTR lpString,
  _In_  int cchString
)
{
  return ::hijack_gdi || ::TextOutA(hdc, nXStart, nYStart, lpString, cchString);
}
BOOL WINAPI MyTextOutW(
  _In_  HDC hdc,
  _In_  int nXStart,
  _In_  int nYStart,
  _In_  LPCWSTR lpString,
  _In_  int cchString
)
{ return ::hijack_gdi || ::TextOutW(hdc, nXStart, nYStart, lpString, cchString); }

BOOL WINAPI MyExtTextOutA(
  _In_  HDC hdc,
  _In_  int X,
  _In_  int Y,
  _In_  UINT fuOptions,
  _In_  const RECT *lprc,
  _In_  LPCSTR lpString,
  _In_  UINT cbCount,
  _In_  const INT *lpDx
)
{ return ::hijack_gdi || ::ExtTextOutA(hdc, X,Y, fuOptions, lprc, lpString, cbCount, lpDx); }
BOOL WINAPI MyExtTextOutW(
  _In_  HDC hdc,
  _In_  int X,
  _In_  int Y,
  _In_  UINT fuOptions,
  _In_  const RECT *lprc,
  _In_  LPCWSTR lpString,
  _In_  UINT cbCount,
  _In_  const INT *lpDx
)
{ return ::hijack_gdi || ::ExtTextOutW(hdc, X,Y, fuOptions, lprc, lpString, cbCount, lpDx); }

DWORD WINAPI MyGetGlyphOutlineA(
  _In_   HDC hdc,
  _In_   UINT uChar,
  _In_   UINT uFormat,
  _Out_  LPGLYPHMETRICS lpgm,
  _In_   DWORD cbBuffer,
  _Out_  LPVOID lpvBuffer,
  _In_   const MAT2 *lpmat2
)
{
  return ::hijack_gdi
      ? ::GetGlyphOutlineA(hdc, ' ', uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2)
      : ::GetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
}
DWORD WINAPI MyGetGlyphOutlineW(
  _In_   HDC hdc,
  _In_   UINT uChar,
  _In_   UINT uFormat,
  _Out_  LPGLYPHMETRICS lpgm,
  _In_   DWORD cbBuffer,
  _Out_  LPVOID lpvBuffer,
  _In_   const MAT2 *lpmat2
)
{
  return ::hijack_gdi
      ? ::GetGlyphOutlineW(hdc, L' ', uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2)
      : ::GetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
}

// EOF

/* TODO: Support more GDI functions
BOOL WINAPI MyTextOutA(
  _In_  HDC hdc,
  _In_  int nXStart,
  _In_  int nYStart,
  _In_  LPCSTR lpString,
  _In_  int cchString
)
{
  growl::debug(QString::fromLocal8Bit(lpString, cchString) + "," + QString::fromLocal8Bit(lpString));
  ::_hdc = hdc;
  :: _nXStart = nXStart;
  :: _nYStart = nYStart;
  return TRUE;
  //POINT pt = {};
  //SIZE sz = {};
  //XFORM xf = {};
  // http://wenku.baidu.com/view/a07c062abd64783e09122be4.html
  //HWND hwnd = ::WindowFromDC(hdc);
  //if (::GetCurrentPositionEx(hdc, &pt)) {
  //if (::GetDCOrgEx(hdc, &pt)) {
  //if (::GetViewportOrgEx(hdc, &pt)) {
  //if (::GetViewportExtEx(hdc, &sz)) {
  //if (::GetWindowOrgEx(hdc, &pt)) {
  //if (::GetWindowExtEx(hdc, &sz)) {
  //if (::GetWorldTransform(hdc, &xf))
  //RECT rc = {};
  //::GetClipBox(hdc, &rc);
  //HRGN rgn = ::CreateRectRgn(0, 0, 0, 0);
  //BOOL ok = ::GetClipRgn(hdc, rgn);
  //growl::debug(QString::number((DWORD)hdc, 16) + ":"
  //           + QString::number(nXStart) + "," + QString::number(nYStart) + ":"
  //           + QString::number(ok) + ":"
  //           //+ QString::number(rc.top) + "," + QString::number(rc.right) + "," + QString::number(rc.bottom) + "," + QString::number(rc.left) + ":"
  //           //+ QString::number(pt.x) + "," + QString::number(pt.y) + ":"
  //           //+ QString::number(sz.cx) + "," + QString::number(sz.cy) + ":"
  //           + QString::fromLocal8Bit(lpString, cchString));
  //lpString = "a" ;
  //cchString = ::strlen(lpString);
  return ::TextOutA(hdc, nXStart, nYStart, lpString, cchString);
}

#define INVISIBLE_FONT_A    "invisible"
#define INVISIBLE_FONT_W    L"invisible"

HFONT WINAPI MyCreateFontIndirectA(
  _In_  const LOGFONTA *lplf
)
{
  if (::hijack_gdi && lplf) {
    LOGFONTA lf;
    memcpy(&lf, lplf, sizeof(lf));
    ::strcpy(lf.lfFaceName, INVISIBLE_FONT_A);
    return ::CreateFontIndirectA(lplf);
  } else
    return ::CreateFontIndirectA(lplf);
}

HFONT WINAPI MyCreateFontIndirectW(
  _In_  const LOGFONTW *lplf
)
{
  if (::hijack_gdi && lplf) {
    LOGFONTW lf;
    memcpy(&lf, lplf, sizeof(lf));
    ::wcscpy(lf.lfFaceName, INVISIBLE_FONT_W);
    return ::CreateFontIndirectW(lplf);
  } else
    return ::CreateFontIndirectW(lplf);
}

int WINAPI MyDrawTextA(
  _In_     HDC hDC,
  _Inout_  LPCSTR lpchText,
  _In_     int nCount,
  _Inout_  LPRECT lpRect,
  _In_     UINT uFormat
)
{ return ::hijack_gdi ? 0 : ::DrawTextA(hDC, lpchText, nCount, lpRect, uFormat); }
int WINAPI MyDrawTextW(
  _In_     HDC hDC,
  _Inout_  LPCWSTR lpchText,
  _In_     int nCount,
  _Inout_  LPRECT lpRect,
  _In_     UINT uFormat
)
{ return ::hijack_gdi ? 0 : ::DrawTextW(hDC, lpchText, nCount, lpRect, uFormat); }

int WINAPI MyDrawTextExA(
  _In_     HDC hdc,
  _Inout_  LPSTR lpchText,
  _In_     int cchText,
  _Inout_  LPRECT lprc,
  _In_     UINT dwDTFormat,
  _In_     LPDRAWTEXTPARAMS lpDTParams
)
{ return ::hijack_gdi ? 0 : ::DrawTextExA(hdc, lpchText, cchText, lprc, dwDTFormat, lpDTParams); }
int WINAPI MyDrawTextExW(
  _In_     HDC hdc,
  _Inout_  LPWSTR lpchText,
  _In_     int cchText,
  _Inout_  LPRECT lprc,
  _In_     UINT dwDTFormat,
  _In_     LPDRAWTEXTPARAMS lpDTParams
)
{ return ::hijack_gdi ? 0 : ::DrawTextExW(hdc, lpchText, cchText, lprc, dwDTFormat, lpDTParams); }

*/
