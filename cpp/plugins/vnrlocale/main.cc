// main.cc
// 5/15/2014

#include "config.h"
#include "winsinglemutex/winsinglemutex.h"
#include "cc/ccmacro.h"
#include <windows.h>

// - Hook functions -
// TO BE REMOVE TO DIFF FILES

LANGID __stdcall MyGetSystemDefaultLangID()
{
  return 0x411;
}

#include "windbg/hijack.h"
bool hijack()
{
  HMODULE hModule = ::GetModuleHandleA(nullptr);
  return WinDbg::overrideFunctionA(hModule, "kernel32.dll", "GetSystemDefaultLangID", MyGetSystemDefaultLangID);
}

// - Main -

BOOL WINAPI DllMain(_In_ HINSTANCE hInstance, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved)
{
  CC_UNUSED(lpvReserved);
  switch (fdwReason) {
  case DLL_PROCESS_ATTACH:
    if (!WinSingleMutex::acquire("vnrlocale"))
      //growl::error("already injected");
      return FALSE;

    ::DisableThreadLibraryCalls(hInstance); // Disable DLL_THREAD_ATTACH and DLL_THREAD_DETACH notifications

    hijack();
    break;

  case DLL_PROCESS_DETACH:
    //Loader::destroy();
    break;
  }
  //SK_ASSERT(0);
  return TRUE;
}

// EOF
