// hookutil_p.cc
// 5/29/2015 jichi
#include "winhook/hookutil_p.h"
#include "disasm/disasm.h"
#include <windows.h>

WINHOOK_BEGIN_NAMESPACE

/**
 *  Overwrite data at the target with the source data in the code region.
 *  @param  dst  target address to modify
 *  @param  src  address of the source data to copy
 *  @param  size  size of the source data to copy
 *  @return   if success
 */
bool detail::protected_memcpy(void *dst, const void *src, size_t size)
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

void detail::move_code(void *code, size_t codesize, ulong oldaddr, ulong newaddr)
{
  BYTE *begin = (BYTE *)code,
       *end = (BYTE *)code + codesize;
  size_t size = 0;
  for (BYTE *p = begin; p < end; p += size) {
    size = ::disasm(p);
    if (!size || p + size > end) // failed to decode instruction
      return;
    if (size == 5 && (*p == s1_jmp_ || *p == s1_call_))
      *(DWORD *)(p + 1) += oldaddr - newaddr;
  }
}

WINHOOK_END_NAMESPACE

// EOF
