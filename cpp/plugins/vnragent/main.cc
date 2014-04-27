// main.cc
// 1/27/2013
// No Qt can appear in this file!
//
// It is very important to restrict from accessing Qt in this file, and make sure the instance to start with wintimer.
// Qt must have the same internal timer with the program's main window.

#include "winquery/winquery.h"
#include "wintimer/wintimer.h"
#include "singleapp/singleapp.h"
#include "loader.h"
#include "cc/ccmacro.h"

//#include "engine/majiro.h"

namespace { // unnamed

HWND waitForWindowReady(int retryCount = 100, int sleepInterval = 100) // retry for 10 seconds
{
  for (int i = 0; i < retryCount; i++) {
    if (HWND winId = WinQuery::getAnyWindowInCurrentProcess())
      return winId;
    ::Sleep(sleepInterval);
  }
  return nullptr;
}

void harakiri() { ::TerminateProcess(::GetCurrentProcess(), EXIT_SUCCESS); }

} // unnamed namespace

// - Main -

BOOL WINAPI DllMain(_In_ HINSTANCE hInstance, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved)
{
  CC_UNUSED(lpvReserved);
  switch (fdwReason) {
  case DLL_PROCESS_ATTACH:
    if (!::singleapp()) {
      //growl::error("already injected");
      return FALSE;
    }

    ::DisableThreadLibraryCalls(hInstance); // Disable DLL_THREAD_ATTACH and DLL_THREAD_DETACH notifications

    if (HWND winId = waitForWindowReady())
      WinTimer::setGlobalWindow(winId);
    else {
      //growl::error("cannot find window");
      return FALSE;
    }

    //while(!::GetModuleHandleA("gdi32.dll"))
    //  ::Sleep(200);
    //while(!::GetModuleHandleA("d3d9.dll"))
    //  ::Sleep(200);
    //::CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(d3dhook), nullptr, 0, nullptr);

    // It is critical to launch Qt application in the same thread as main window
    WinTimer::singleShot(100, boost::bind(Loader::initWithInstance, hInstance));

    //My::OverrideGDIModules();
    //if (auto eng = Engine::getEngine()) {
    //  Engine::setEnabled(true);
    //  eng->inject();
    //}
    break;

  case DLL_PROCESS_DETACH:
    WinTimer::singleShot(5000, harakiri);  // If hang, terminate the process in 5 seconds.
    Loader::destroy();
    break;
  }
  //SK_ASSERT(0);
  return TRUE;
}

// EOF
