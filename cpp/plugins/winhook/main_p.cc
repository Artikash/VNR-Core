// main_p.cc
// 1/27/2013

#include "winhook/main.h"
#include "winhook/main_p.h"
#include "winhook/myfunc.h"
#include "winhook/qt/mydef.h"
#include "winhook/qt/mainobj.h"
#include "windbg/inject.h"
#include "windbg/util.h"
#include "cc/ccmacro.h"

#ifdef _MSC_VER
# pragma warning (disable:4996)   // C4996: use POSIX function (stricmp)
#endif // _MSC_VER

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

// - Construction -

namespace { namespace detail { MainPrivate *d_ptr = nullptr; }}

void Main::initWithInstance(HINSTANCE hInstance)
{
#ifdef WITH_LIB_WINHOOK
  ::GetModuleFileNameW(hInstance, MODULE_PATH, MAX_PATH);
#else
  CC_UNUSED(hInstance);
#endif // WITH_LIB_WINHOOK

  MainObject::init();
  detail::d_ptr = new MainPrivate;
}

void Main::destroy()
{
  delete detail::d_ptr;
  detail::d_ptr = nullptr;
  MainObject::destroy();
}

MainPrivate::MainPrivate()
{
  WinDbg::ThreadsSuspender suspendedThreads; // lock all threads
  My::OverrideModules();

  rehookTimer.setInterval(My::EventLoopTimeout * 10);
  rehookTimer.setFunction(&My::OverrideModules);

  retransTimer.setInterval(My::EventLoopTimeout);
  retransTimer.setFunction(boost::bind(&MainObject::updateProcessWindows, 0));

#ifdef WITH_LIB_WINHOOK
  // FIXME: Enable this after metacall supports multiple clients
  WinHook::hookAfterFunction(::CreateProcessW, L"", nullptr, PostCreateProcessW);
  WinHook::hookAfterFunction(::CreateProcessA, L"", nullptr, PostCreateProcessA);
#endif // WITH_LIB_WINHOOK

  retransTimer.start();
  rehookTimer.start();
}

MainPrivate::~MainPrivate()
{
  retransTimer.stop();
  rehookTimer.stop();
}

// EOF

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
