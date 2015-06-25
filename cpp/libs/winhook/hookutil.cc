// hookutil.cc
// 5/29/2015 jichi
#include "winhook/hookutil.h"
#include "disasm/disasm.h"
#include "winasm/winasmdef.h"
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

bool csmemset(void *dst, byte value, size_t size)
{
  DWORD oldProtect;
  if (!::VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect))
    return false;
  ::memset(dst, value, size);
  DWORD newProtect;
  ::VirtualProtect(dst, size, oldProtect, &newProtect); // the error code is not checked for this function
  return true;
}

bool remove_inst(ulong addr)
{
  auto p = (LPVOID)addr;
  size_t size = ::disasm(p);
  return size && csmemset(p, s1_nop, size);
}

WINHOOK_END_NAMESPACE

// EOF
