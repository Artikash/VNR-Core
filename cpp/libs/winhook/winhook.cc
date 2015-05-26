// winhook.cc
// 5/25/2015 jichi
#include "winhook/winhook.h"
#include <windows.h>

namespace { // unnamed

// Helper functions

/**
 *  Overwrite data at the target with the source data in the code region.
 *  @param  dst  target address to modify
 *  @param  src  address of the source data to copy
 *  @param  size  size of the source data to copy
 *  @return   if success
 */
bool protected_memcpy(void *dst, const void *src, size_t size)
{
  DWORD pid = ::GetCurrentProcessId();
  DWORD oldProtect;
  HANDLE hProc = ::OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE, FALSE, pid);
  if (!hProc)
    return false;
  if (!::VirtualProtectEx(hProc, dst, size, PAGE_EXECUTE_READWRITE, &oldProtect))
    return false;
  ::memcpy(dst, src, size);
  DWORD newProtect;
  ::VirtualProtectEx(hProc, dst, size, oldProtect, &newProtect); // the error code is not checked for this function
  return true;
}

// Hook manager

class HookManager
{
public:
  bool unhook(DWORD address)
  {
    // Not implemented
    return false;
  }

  bool hook(DWORD address, winhook::hook_fun_t callback)
  {
    return true;
  }
};

HookManager *hookManager;
HookManager *createHookManager() { return new HookManager; }

} // unnamed namespace

WINHOOK_BEGIN_NAMESPACE

bool hook(ulong address, hook_fun_t callback)
{
  if (!::hookManager)
    ::hookManager = ::createHookManager();
  return ::hookManager->hook(address, callback);
}

bool unhook(ulong address)
{ return ::hookManager && ::hookManager->unhook(address); }

WINHOOK_END_NAMESPACE

// EOF
