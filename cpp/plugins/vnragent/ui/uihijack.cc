// uihijack.cc
// 1/27/2013 jichi
#include "ui/uihijack.h"
#include "ui/uihijack_p.h"
#include "windbg/hijack.h"
#include "winiter/winiterps.h"
#include <boost/foreach.hpp>

#ifdef _MSC_VER
# pragma warning (disable:4996)   // C4996: use POSIX function (wcscat)
#endif // _MSC_VER

// - Helpers -

namespace { // unnamed

const Ui::FunctionInfo UI_HIJACK_FUNCTIONS[] = { UI_HIJACK_FUNCTIONS_INITIALIZER };

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

void Ui::overrideModules()
{
  LPCWSTR exeName = applicationNameW(),
          exePath = applicationPathW();
  if (!exeName || !exePath) {
    //growl::debug("failed to get application name or path");
    return;
  }

  WCHAR path[MAX_PATH];
  WinIter::iterProcessModules([&](HMODULE hModule) {
    if (::GetModuleFileNameW(hModule, path, MAX_PATH) &&
        !::wcsnicmp(path, exePath, exeName - exePath))
      overrideModuleFunctions(hModule);
  });
}

void Ui::overrideModuleFunctions(LPVOID pModule)
{
  HMODULE hModule = (HMODULE)pModule;
  BOOST_FOREACH (const FunctionInfo &fn, UI_HIJACK_FUNCTIONS) {
    PVOID ret = WinDbg::overrideFunctionA(hModule, fn.moduleName, fn.functionName, fn.functionAddress);
    if (ret) {
      //growl::debug(fn.functionName); // success
    }
  }
}

// - My Functions -

HMODULE WINAPI Ui::MyLoadLibrary(_In_ LPCTSTR lpFileName)
{
  HMODULE ret = ::LoadLibrary(lpFileName);
  if (!::GetModuleHandle(lpFileName)) // this is the first load
    Ui::overrideModuleFunctions(ret);
  return ret;
}

HMODULE WINAPI Ui::MyLoadLibraryEx(_In_ LPCTSTR lpFileName, __reserved HANDLE hFile, _In_ DWORD dwFlags)
{
  HMODULE ret = ::LoadLibraryEx(lpFileName, hFile, dwFlags);
  if (!::GetModuleHandle(lpFileName)) // this is the first load
    Ui::overrideModuleFunctions(ret);
  return ret;
}

LPVOID WINAPI Ui::MyGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
  char modulePath[MAX_PATH];
  if (::GetModuleFileNameA(hModule, modulePath, MAX_PATH)) {
    const char *moduleName = ::basename(modulePath);
    BOOST_FOREACH (const FunctionInfo &fn, UI_HIJACK_FUNCTIONS)
      if (!::stricmp(moduleName, fn.moduleName) && !::stricmp(lpProcName, fn.functionName))
        return fn.functionAddress;
  }
  return ::GetProcAddress(hModule, lpProcName);
}

// EOF

#ifdef WITH_LIB_WINHOOK
# include "winhook/funchook.h"

namespace { // unnamed

wchar_t MODULE_PATH[MAX_PATH]; // dll path

// - Helpers -

inline bool InjectDll(HANDLE hProcess)
{ return WinDbg::injectDllW(MODULE_PATH, 0, hProcess); }

// - Callbacks -

BOOL CALLBACK PostCreateProcessW(
  BOOL exitCode,
  _In_opt_     LPCWSTR lpApplicationName,
  _Inout_opt_  LPWSTR lpCommandLine,
  _In_opt_     LPSECURITY_ATTRIBUTES lpProcessAttributes,
  _In_opt_     LPSECURITY_ATTRIBUTES lpThreadAttributes,
  _In_         BOOL bInheritHandles,
  _In_         DWORD dwCreationFlags,
  _In_opt_     LPVOID lpEnvironment,
  _In_opt_     LPCWSTR lpCurrentDirectory,
  _In_         LPSTARTUPINFOW lpStartupInfo,
  _Out_        LPPROCESS_INFORMATION lpProcessInformation
  )
{
  CC_UNUSED(lpApplicationName);
  CC_UNUSED(lpCommandLine);
  CC_UNUSED(lpProcessAttributes);
  CC_UNUSED(lpThreadAttributes);
  CC_UNUSED(bInheritHandles);
  CC_UNUSED(dwCreationFlags);
  CC_UNUSED(lpEnvironment);
  CC_UNUSED(lpCurrentDirectory);
  CC_UNUSED(lpStartupInfo);
  // Note: exitCode should be 0 when lpProcessInformation is 0, anyways.  It's a required parameter.
  if (exitCode && lpProcessInformation)
    InjectDll(lpProcessInformation->hProcess);
  return exitCode;
}

BOOL CALLBACK PostCreateProcessA(
  BOOL exitCode,
  _In_opt_     LPCSTR lpApplicationName,
  _Inout_opt_  LPSTR lpCommandLine,
  _In_opt_     LPSECURITY_ATTRIBUTES lpProcessAttributes,
  _In_opt_     LPSECURITY_ATTRIBUTES lpThreadAttributes,
  _In_         BOOL bInheritHandles,
  _In_         DWORD dwCreationFlags,
  _In_opt_     LPVOID lpEnvironment,
  _In_opt_     LPCSTR lpCurrentDirectory,
  _In_         LPSTARTUPINFOA lpStartupInfo,
  _Out_        LPPROCESS_INFORMATION lpProcessInformation
  )
{
  CC_UNUSED(lpApplicationName);
  CC_UNUSED(lpCommandLine);
  CC_UNUSED(lpProcessAttributes);
  CC_UNUSED(lpThreadAttributes);
  CC_UNUSED(bInheritHandles);
  CC_UNUSED(dwCreationFlags);
  CC_UNUSED(lpEnvironment);
  CC_UNUSED(lpCurrentDirectory);
  CC_UNUSED(lpStartupInfo);
  // Note: exitCode should be 0 when lpProcessInformation is 0, anyways.  It's a required parameter.
  if (exitCode && lpProcessInformation)
    InjectDll(lpProcessInformation->hProcess);
  return exitCode;
}
} // unnamed namespace
#endif // WITH_LIB_WINHOOK

/*
  DWORD WINAPI ThreadProc(LPVOID params)
  {
    CC_UNUSED(params);
    DWORD pid = ::GetCurrentProcessId();
    DWORD tid = ::GetCurrentThreadId();
    //Buffer buffer = {0,0};
    //buffer.size = 3000;
    //buffer.text = (wchar_t *)malloc(sizeof(wchar_t) * 3000);
    int idle = 0;
    int sleepTime = 500;
    int checkWindowInterval = 500;
    int checkWindowTime = 0;
    int rehookInterval = 10000;
    int rehookTime = 5000;
    int t = GetTickCount();
    while (true) {
      int oldt = t;
      t = GetTickCount();
      int dt = t - oldt;
      if (MainPrivate::globalInstance()->aboutToQuit)
        break;
      //CheckDrawing(t);
      rehookTime -= dt;
      if (rehookTime <= 0) {
        OverrideModules();
        rehookTime = rehookInterval;
      }
      checkWindowTime -= dt;
      if (checkWindowTime <= 0) { // && (settings.injectionFlags & TRANSLATE_MENUS)) {
        checkWindowTime = checkWindowInterval;
        HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (h == INVALID_HANDLE_VALUE)
          break;
        THREADENTRY32 thread;
        thread.dwSize = sizeof(thread);
        int count = 0;
        int windows = 0;
        if (Thread32First(h, &thread)) {
          do {
            if (thread.th32OwnerProcessID != pid || thread.th32ThreadID == tid)
              continue;
            //windows |= TranslateThreadWindows(thread.th32ThreadID, buffer);
            windows |= TranslateThreadWindows(thread.th32ThreadID);
            count++;
          }
          while (Thread32Next(h, &thread));
        }
        CloseHandle(h);
        if (!count) break;
        if (!windows) {
          idle++;
          if (idle >= 40)
            break;
        }
      }
      Sleep(sleepTime);
    }
    // Never seem to end up here, for some reason.
    //free(buffer.text);
    //UninitAtlas();
    return 0;
  }

*/
