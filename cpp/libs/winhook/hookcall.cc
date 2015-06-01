// hookcall.cc
// 5/31/2015 jichi
#include "winhook/hookcall.h"
#include "winhook/hookutil_p.h"
#include <windows.h>

WINHOOK_BEGIN_NAMESPACE

ulong replace_call(ulong addr, ulong newop)
{
  ulong oldop = addr + jmp_ins_size + *(DWORD *)addr;
  DWORD val = newop - addr - jmp_ins_size;
  if (!detail::protected_memcpy((LPVOID)addr, &val, sizeof(DWORD)))
    return 0;
  return oldop;
}

WINHOOK_END_NAMESPACE

// EOF
