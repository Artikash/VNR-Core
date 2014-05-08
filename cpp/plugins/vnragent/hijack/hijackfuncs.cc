// hijackfuncs.cc
// 1/27/2013 jichi
#include "hijack/hijackfuncs.h"
#include "hijack/hijackfuncs_p.h"
#include "windbg/hijack.h"
#include "winiter/winiterps.h"
#include <boost/foreach.hpp>

#ifdef _MSC_VER
# pragma warning (disable:4996)   // C4996: use POSIX function (wcsnimp)
#endif // _MSC_VER

// - Helpers -

namespace { // unnamed

const Hijack::FunctionInfo HIJACK_FUNCTIONS[] = { HIJACK_FUNCTIONS_INITIALIZER };

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

// API

void Hijack::overrideModules()
{
  LPCWSTR exeName = applicationNameW(),
          exePath = applicationPathW();
  if (!exeName || !exePath) {
    //growl::debug("failed to get application name or path");
    return;
  }

  WCHAR path[MAX_PATH];
  WinIter::iterProcessModules([=, &path](HMODULE hModule) {
    if (::GetModuleFileNameW(hModule, path, MAX_PATH) &&
        !::wcsnicmp(path, exePath, exeName - exePath))
      overrideModuleFunctions(hModule);
  });
}

void Hijack::restoreModules()
{
  LPCWSTR exeName = applicationNameW(),
          exePath = applicationPathW();
  if (!exeName || !exePath)
    return;

  WCHAR path[MAX_PATH];
  WinIter::iterProcessModules([=, &path](HMODULE hModule) {
    if (::GetModuleFileNameW(hModule, path, MAX_PATH) &&
        !::wcsnicmp(path, exePath, exeName - exePath))
      restoreModuleFunctions(hModule);
  });
}

void Hijack::overrideModuleFunctions(HMODULE hModule)
{
  BOOST_FOREACH (const auto &fn, HIJACK_FUNCTIONS)
    if (PVOID ret = WinDbg::overrideFunctionA(hModule, fn.moduleName, fn.functionName, fn.newFunctionAddress)) {
      //growl::debug(fn.functionName); // success
    }
}

void Hijack::restoreModuleFunctions(HMODULE hModule)
{
  BOOST_FOREACH (const auto &fn, HIJACK_FUNCTIONS)
    WinDbg::overrideFunctionA(hModule, fn.moduleName, fn.functionName, fn.oldFunctionAddress);
}

LPVOID Hijack::getOverridingFunctionAddress(HMODULE hModule, LPCSTR lpProcName)
{
  char modulePath[MAX_PATH];
  if (::GetModuleFileNameA(hModule, modulePath, MAX_PATH))
    if (const char *moduleName = ::basename(modulePath))
      BOOST_FOREACH (const auto &fn, HIJACK_FUNCTIONS)
        if (!::stricmp(moduleName, fn.moduleName) && !::stricmp(lpProcName, fn.functionName))
          return fn.newFunctionAddress;
  return nullptr;
}

// EOF
