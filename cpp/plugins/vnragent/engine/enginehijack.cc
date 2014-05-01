// enginehijack.cc
// 1/27/2013 jichi
#include "engine/enginehijack.h"
#include "engine/enginehijack_p.h"
#include "windbg/hijack.h"
#include "winiter/winiterps.h"
#include <boost/foreach.hpp>

#ifdef _MSC_VER
# pragma warning (disable:4996)   // C4996: use POSIX function (wcscat)
#endif // _MSC_VER

// - Helpers -

namespace { // unnamed

const Engine::FunctionInfo ENGINE_HIJACK_FUNCTIONS[] = { ENGINE_HIJACK_FUNCTIONS_INITIALIZER };

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

// - Hijack -

void Engine::overrideModules()
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

void Engine::overrideModuleFunctions(HMODULE hModule)
{
  BOOST_FOREACH (const FunctionInfo &fn, ENGINE_HIJACK_FUNCTIONS) {
    PVOID ret = WinDbg::overrideFunctionA(hModule, fn.moduleName, fn.functionName, fn.functionAddress);
    if (ret) {
      //growl::debug(fn.functionName); // success
    }
  }
}

// EOF
