// bgi2.cc
// 5/12/2014 jichi
// See: http://i.watashi.me/archives/1.html
// See: http://bbs.sumisora.org/read.php?tid=11042351
#include "engine/model/bgi.h"
#include "engine/enginedef.h"
#include "engine/engineenv.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>

/**
 *  jichi 2/5/2014: Add an alternative BGI hook
 *
 *  Issue: This hook cannot extract character name for コトバの消えた日
 *
 *  See: http://tieba.baidu.com/p/2845113296
 *  世界と世界の真ん中で
 *  - /HSN4@349E0:sekachu.exe // Disabled BGI3, floating split char
 *  - /HS-1C:-4@68E56 // Not used, cannot detect character name
 *  - /HSC@34C80:sekachu.exe  // BGI2, extract both scenario and character names
 *
 *  [Lump of Sugar] 世界と世界の真ん中で
 *  /HSC@34C80:sekachu.exe
 *  - addr: 216192 = 0x34c80
 *  - module: 3599131534
 *  - off: 12 = 0xc
 *  - type: 65 = 0x41
 *
 *  base: 0x11a0000
 *  hook_addr = base + addr = 0x11d4c80
 *
 *  011d4c7e     cc             int3
 *  011d4c7f     cc             int3
 *  011d4c80  /$ 55             push ebp    ; jichi: hook here
 *  011d4c81  |. 8bec           mov ebp,esp
 *  011d4c83  |. 6a ff          push -0x1
 *  011d4c85  |. 68 e6592601    push sekachu.012659e6
 *  011d4c8a  |. 64:a1 00000000 mov eax,dword ptr fs:[0]
 *  011d4c90  |. 50             push eax
 *  011d4c91  |. 81ec 300d0000  sub esp,0xd30
 *  011d4c97  |. a1 d8c82801    mov eax,dword ptr ds:[0x128c8d8]
 *  011d4c9c  |. 33c5           xor eax,ebp
 *  011d4c9e  |. 8945 f0        mov dword ptr ss:[ebp-0x10],eax
 *  011d4ca1  |. 53             push ebx
 *  011d4ca2  |. 56             push esi
 *  011d4ca3  |. 57             push edi
 *  011d4ca4  |. 50             push eax
 *  011d4ca5  |. 8d45 f4        lea eax,dword ptr ss:[ebp-0xc]
 *  011d4ca8  |. 64:a3 00000000 mov dword ptr fs:[0],eax
 *  011d4cae  |. 8b4d 0c        mov ecx,dword ptr ss:[ebp+0xc]
 *  011d4cb1  |. 8b55 18        mov edx,dword ptr ss:[ebp+0x18]
 *  011d4cb4  |. 8b45 08        mov eax,dword ptr ss:[ebp+0x8]
 *  011d4cb7  |. 8b5d 10        mov ebx,dword ptr ss:[ebp+0x10]
 *  011d4cba  |. 8b7d 38        mov edi,dword ptr ss:[ebp+0x38]
 *  011d4cbd  |. 898d d8f3ffff  mov dword ptr ss:[ebp-0xc28],ecx
 *  011d4cc3  |. 8b4d 28        mov ecx,dword ptr ss:[ebp+0x28]
 *  011d4cc6  |. 8995 9cf3ffff  mov dword ptr ss:[ebp-0xc64],edx
 *  011d4ccc  |. 51             push ecx
 *  011d4ccd  |. 8b0d 305c2901  mov ecx,dword ptr ds:[0x1295c30]
 *  011d4cd3  |. 8985 e0f3ffff  mov dword ptr ss:[ebp-0xc20],eax
 *  011d4cd9  |. 8b45 1c        mov eax,dword ptr ss:[ebp+0x1c]
 *  011d4cdc  |. 8d95 4cf4ffff  lea edx,dword ptr ss:[ebp-0xbb4]
 *  011d4ce2  |. 52             push edx
 *  011d4ce3  |. 899d 40f4ffff  mov dword ptr ss:[ebp-0xbc0],ebx
 *  011d4ce9  |. 8985 1cf4ffff  mov dword ptr ss:[ebp-0xbe4],eax
 *  011d4cef  |. 89bd f0f3ffff  mov dword ptr ss:[ebp-0xc10],edi
 *  011d4cf5  |. e8 862efdff    call sekachu.011a7b80
 *  011d4cfa  |. 33c9           xor ecx,ecx
 *  011d4cfc  |. 8985 60f3ffff  mov dword ptr ss:[ebp-0xca0],eax
 *  011d4d02  |. 3bc1           cmp eax,ecx
 *  011d4d04  |. 0f84 0f1c0000  je sekachu.011d6919
 *  011d4d0a  |. e8 31f6ffff    call sekachu.011d4340
 *  011d4d0f  |. e8 6cf8ffff    call sekachu.011d4580
 *  011d4d14  |. 8985 64f3ffff  mov dword ptr ss:[ebp-0xc9c],eax
 *  011d4d1a  |. 8a03           mov al,byte ptr ds:[ebx]
 *  011d4d1c  |. 898d 90f3ffff  mov dword ptr ss:[ebp-0xc70],ecx
 *  011d4d22  |. 898d 14f4ffff  mov dword ptr ss:[ebp-0xbec],ecx
 *  011d4d28  |. 898d 38f4ffff  mov dword ptr ss:[ebp-0xbc8],ecx
 *  011d4d2e  |. 8d71 01        lea esi,dword ptr ds:[ecx+0x1]
 *  011d4d31  |. 3c 20          cmp al,0x20
 *  011d4d33  |. 7d 75          jge short sekachu.011d4daa
 *  011d4d35  |. 0fbec0         movsx eax,al
 *  011d4d38  |. 83c0 fe        add eax,-0x2                             ;  switch (cases 2..8)
 *  011d4d3b  |. 83f8 06        cmp eax,0x6
 *  011d4d3e  |. 77 6a          ja short sekachu.011d4daa
 *  011d4d40  |. ff2485 38691d0>jmp dword ptr ds:[eax*4+0x11d6938]
 */
static ulong searchBGI2(ulong startAddress, ulong stopAddress)
{
  const BYTE ins[] = {
    0x3c, 0x20,      // 011d4d31  |. 3c 20          cmp al,0x20
    0x7d, 0x75,      // 011d4d33  |. 7d 75          jge short sekachu.011d4daa
    0x0f,0xbe,0xc0,  // 011d4d35  |. 0fbec0         movsx eax,al
    0x83,0xc0, 0xfe, // 011d4d38  |. 83c0 fe        add eax,-0x2                             ;  switch (cases 2..8)
    0x83,0xf8, 0x06, // 011d4d3b  |. 83f8 06        cmp eax,0x6
    0x77, 0x6a       // 011d4d3e  |. 77 6a          ja short sekachu.011d4daa
  };
  enum { hook_offset = 0x34c80 - 0x34d31 }; // distance to the beginning of the function
  ulong range = min(stopAddress - startAddress, Engine::MaximumMemoryRange);
  ulong reladdr = MemDbg::searchPattern(startAddress, range, ins, sizeof(ins));
  if (!reladdr)
    //ConsoleOutput("vnreng:BGI2: pattern not found");
    return 0;

  ulong addr = startAddress + reladdr + hook_offset;
  enum : BYTE { push_ebp = 0x55 };  // 011d4c80  /$ 55             push ebp
  if (*(BYTE *)addr != push_ebp)
    //ConsoleOutput("vnreng:BGI2: pattern found but the function offset is invalid");
    return 0;

  return addr;
}

bool BGIEngine::attachBGIType2()
{
  ulong startAddress,
        stopAddress;
  if (!Engine::getCurrentMemoryRange(&startAddress, &stopAddress))
    return false;
  ulong addr = ::searchBGI2(startAddress, stopAddress);
  //ulong addr = startAddress + 0x31850; // 世界と世界の真ん中 体験版
  bool ok = addr && hookAddress(addr);
  if (ok)
    setName("BGI2"); // change engine name
  return ok;
}

// EOF
