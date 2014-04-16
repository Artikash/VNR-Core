// mygdi.cc
// 4/14/2014 jichi
#include "winhook/hijack/mygdi.h"
#include "winhook/hijack/mygdi_p.h"
//#include "winhook/qt/mainobj.h"
#include "winhook/util/winsec.h"
#include <psapi.h>
#include <boost/foreach.hpp>

#ifdef _MSC_VER
# pragma warning (disable:4996)   // C4996: use POSIX function (wcscat)
#endif // _MSC_VER

//#define DEBUG "mygdi"
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

void My::OverrideGDIModuleFunctions(HMODULE hModule)
{
  BOOST_FOREACH (const MyFunctionInfo &fn, MY_FUNCTIONS) {
#ifdef DEBUG
    PVOID ret = winsec::OverrideFunctionA(hModule, fn.moduleName, fn.functionName, fn.functionAddress);
    if (ret)
      growl::warn(fn.functionName);
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

BOOL gdi_enabled = TRUE; // CHECKPOINT

BOOL WINAPI MyTextOutA(
  __in  HDC hdc,
  __in  int nXStart,
  __in  int nYStart,
  __in  LPCSTR lpString,
  __in  int cchString
)
{
  return ::gdi_enabled || ::TextOutA(hdc, nXStart, nYStart, lpString, cchString);
}

// EOF
