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

DWORD searchPattern(DWORD base, DWORD base_length, LPCVOID search, DWORD search_length) // KMP
{
  __asm
  {
    mov eax,search_length
alloc:
    push 0
    sub eax,1
    jnz alloc

    mov edi,search
    mov edx,search_length
    mov ecx,1
    xor esi,esi
build_table:
    mov al,byte ptr [edi+esi]
    cmp al,byte ptr [edi+ecx]
    sete al
    test esi,esi
    jz pre
    test al,al
    jnz pre
    mov esi,[esp+esi*4-4]
    jmp build_table
pre:
    test al,al
    jz write_table
    inc esi
write_table:
    mov [esp+ecx*4],esi

    inc ecx
    cmp ecx,edx
    jb build_table

    mov esi,base
    xor edx,edx
    mov ecx,edx
matcher:
    mov al,byte ptr [edi+ecx]
    cmp al,byte ptr [esi+edx]
    sete al
    test ecx,ecx
    jz match
    test al,al
    jnz match
    mov ecx, [esp+ecx*4-4]
    jmp matcher
match:
    test al,al
    jz pre2
    inc ecx
    cmp ecx,search_length
    je finish
pre2:
    inc edx
    cmp edx,base_length // search_length
    jb matcher
    mov edx,search_length
    dec edx
finish:
    mov ecx,search_length
    sub edx,ecx
    lea eax,[edx+1]
    lea ecx,[ecx*4]
    add esp,ecx
  }
}

#if 0
// jichi 2/5/2014: '?' = 0xff
// See: http://sakuradite.com/topic/124
DWORD searchPatternEx(DWORD base, DWORD base_length, LPCVOID search, DWORD search_length, BYTE wildcard) // KMP
{
  __asm
  {
    // jichi 2/5/2014 BEGIN
    mov bl,wildcard
    // jichi 2/5/2014 END
    mov eax,search_length
alloc:
    push 0
    sub eax,1
    jnz alloc // jichi 2/5/2014: this will also set %eax to zero

    mov edi,search
    mov edx,search_length
    mov ecx,1
    xor esi,esi
build_table:
    mov al,byte ptr [edi+esi]
    cmp al,byte ptr [edi+ecx]
    sete al
    test esi,esi
    jz pre
    test al,al
    jnz pre
    mov esi,[esp+esi*4-4]
    jmp build_table
pre:
    test al,al
    jz write_table
    inc esi
write_table:
    mov [esp+ecx*4],esi

    inc ecx
    cmp ecx,edx
    jb build_table

    mov esi,base
    xor edx,edx
    mov ecx,edx
matcher:
    mov al,byte ptr [edi+ecx] // search
    // jichi 2/5/2014 BEGIN
    mov bh,al // save loaded byte to reduce cache access. %ah is not used and always zero
    cmp al,bl // %bl is the wildcard byte
    sete al
    test al,al
    jnz wildcard_matched
    mov al,bh // restore the loaded byte
    // jichi 2/5/2014 END
    cmp al,byte ptr [esi+edx] // base
    sete al
    // jichi 2/5/2014 BEGIN
wildcard_matched:
    // jichi 2/5/2014 END
    test ecx,ecx
    jz match
    test al,al
    jnz match
    mov ecx, [esp+ecx*4-4]
    jmp matcher
match:
    test al,al
    jz pre2
    inc ecx
    cmp ecx,search_length
    je finish
pre2:
    inc edx
    cmp edx,base_length // search_length
    jb matcher
    mov edx,search_length
    dec edx
finish:
    mov ecx,search_length
    sub edx,ecx
    lea eax,[edx+1]
    lea ecx,[ecx*4]
    add esp,ecx
  }
}

#endif // 0

MEMDBG_END_NAMESPACE

// EOF
