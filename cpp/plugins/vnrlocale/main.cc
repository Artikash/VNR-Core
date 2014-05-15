// main.cc
// 5/15/2014

#include "config.h"
#include "singleapp/singleapp.h"
#include "cc/ccmacro.h"
#include <windows.h>

// - Main -

BOOL WINAPI DllMain(_In_ HINSTANCE hInstance, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved)
{
  CC_UNUSED(lpvReserved);
  switch (fdwReason) {
  case DLL_PROCESS_ATTACH:
    if (!::singleapp())
      //growl::error("already injected");
      return FALSE;

    ::DisableThreadLibraryCalls(hInstance); // Disable DLL_THREAD_ATTACH and DLL_THREAD_DETACH notifications
    break;

  case DLL_PROCESS_DETACH:
    //Loader::destroy();
    break;
  }
  //SK_ASSERT(0);
  return TRUE;
}

// EOF
