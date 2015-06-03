// hookutil.cc
// 5/29/2015 jichi
#include "winhook/hookutil.h"
#include <windows.h>

WINHOOK_BEGIN_NAMESPACE

bool csmemcpy(void *dst, const void *src, size_t size)
{
  DWORD oldProtect;
  //DWORD pid = ::GetCurrentProcessId();
  //HANDLE hProc = ::OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE, false, pid);
  //if (!hProc)
  //  return false;
  //if (!::VirtualProtectEx(hProc, dst, size, PAGE_EXECUTE_READWRITE, &oldProtect))
  if (!::VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect))
    return false;
  ::memcpy(dst, src, size);
  DWORD newProtect;
  ::VirtualProtect(dst, size, oldProtect, &newProtect); // the error code is not checked for this function
  return true;
}

WINHOOK_END_NAMESPACE

// EOF
