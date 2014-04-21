// memsearch.cc
// 4/20/2014 jichi
#include "memdbg/memsearch.h"
#include <windows.h>

// Helpers

namespace { // unnamed

// jichi 4/19/2014: Return the integer that can mask the signature
DWORD SigMask(DWORD sig)
{
  __asm
  {
    xor ecx,ecx
    mov eax,sig
_mask:
    shr eax,8
    inc ecx
    test eax,eax
    jnz _mask
    sub ecx,4
    neg ecx
    or eax,-1
    shl ecx,3
    shr eax,cl
  }
}

} // namespace unnamed

MEMDBG_BEGIN_NAMESPACE

DWORD findCallerAddress(DWORD funcAddr, DWORD sig, DWORD lowerBound, DWORD upperBound, DWORD reverse_length)
{
  DWORD  size = upperBound - lowerBound;
  DWORD fun = (DWORD)funcAddr;
  //WCHAR str[0x40];
  DWORD mask = SigMask(sig);
  for (DWORD i = 0x1000; i < size - 4; i++)
    if (*(WORD *)(lowerBound + i) == 0x15ff) {
      DWORD t = *(DWORD *)(lowerBound + i + 2);
      if (t >= lowerBound && t <= upperBound - 4) {
        if (*(DWORD *)t == fun)
          //swprintf(str,L"CALL addr: 0x%.8X",lowerBound + i);
          //OutputConsole(str);
          for (DWORD j = i ; j > i - reverse_length; j--)
            if ((*(DWORD *)(lowerBound + j) & mask) == sig) // Fun entry 1.
              //swprintf(str,L"Entry: 0x%.8X",lowerBound + j);
              //OutputConsole(str);
              return lowerBound + j;

      } else
        i += 6;
    }
  //OutputConsole(L"Find call and entry failed.");
  return 0;
}

MEMDBG_END_NAMESPACE

// EOF
