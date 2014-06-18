// profile_bgi.cc
// 6/15/2014
// Currently, only BGI is implemented

#include "profile.h"
#include "windbg/hijack.h"
#include <windows.h>

namespace { namespace BGI {

enum { LCID = 0x411 };

// Return 0x411 on the first call
LANGID WINAPI myGetSystemDefaultLangID()
{
  //return LCID;
  static bool once = false;
  if (!once) {
    once = true;
    return LCID;
  } else
    return ::GetSystemDefaultLangID();
}

}} // unnamed namespace BGI

void Profile::useBGI()
{
  if (::GetSystemDefaultLangID() != BGI::LCID) {
    HMODULE hModule = ::GetModuleHandleA(nullptr);
    WinDbg::overrideFunctionA(hModule, "kernel32.dll", "GetSystemDefaultLangID", BGI::myGetSystemDefaultLangID);
  }
}

// EOF
