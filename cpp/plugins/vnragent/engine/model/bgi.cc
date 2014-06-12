// bgi.cc
// 5/11/2014 jichi
#include "engine/model/bgi.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>

/**
 *  Type 1: Sample game: FORTUNE ARTERIAL, 0x4207E0
 *
 *  ? __cdecl sub_4207E0 proc near
 *  - arg1: address
 *  - arg2: address
 *  - arg3: string, LPCSTR
 *  - arg4: 0
 *  - arg5: function address
 *  - arg6: address, pointed to 0
 *  - arg7: address, pointed to 0
 *  - arg8: 0x1c, 0x18, observation from FA: This value diff for scenario and name
 *  - arg9: 1
 *  - arg10: 0
 *  - arg11: 0 or 1, maybe, gender?
 *  - arg12: 0x00ffffff
 *  - arg13: addr
 *
 *  Guessed function signature:
 *  typedef int (__cdecl *hook_fun_t)(DWORD, DWORD, LPCSTR, DWORD, DWORD, DWORD,
 *                                    DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD);
 *  static int __cdecl newHookFun(DWORD arg1, DWORD arg2, LPCSTR str, DWORD arg4, DWORD arg5, DWORD arg6,
 *                                DWORD arg7, DWORD split, DWORD arg9, DWORD arg10, DWORD arg11, DWORD arg12, DWORD arg13)
 *
 *  Type 2: Sample game: 世界と世界の真ん中 体験版, relative address: 0x31850;
 *
 *  ? __cdecl sub_31850(DWORD arg1, DWORD arg2, LPCSTR arg3, DWORD arg4)
 *  - arg1: address, could point to area of zero, seems to be the output of this function
 *  - arg2: address, the same as arg1
 *  - arg3: string
 *  - arg4: flags, choices: 0, character name & scenario: 1
 *  - return: not sure, seems always to be 0 when success
 *
 *  Guessed function signature:
 *  - typedef int (__cdecl *hook_fun_t)(DWORD, DWORD, LPCSTR, DWORD); // __stdcall will crash the game
 *  - static int __cdecl newHookFun(DWORD arg1, DWORD arg2, LPCSTR str, DWORD split)
 */
void BGIEngine::hook(HookStack *stack)
{
  static QByteArray data_; // persistent storage, which makes this function not thread-safe

  LPCSTR text3 = (LPCSTR)stack->args[2]; // arg3

  // In Type 1, split = arg8
  // In Type 2, there is no arg8. However, arg8 seems to be a good split that can differenciate choice and character name
  //DWORD split = stack->args[3]; // arg4
  DWORD split = stack->args[7]; // arg8

  auto sig = Engine::hashThreadSignature(stack->retaddr, split);

  data_ = EngineController::instance()->dispatchTextA(text3, sig, Engine::UnknownRole);
  stack->args[2] = (DWORD)data_.constData();
}

/**
 *  5/12/2014
 *  This is the caller of the ITH BGI hook, which extract text by characters
 *  and cannot be used for substition.
 *
 *  Sample game: 世界征服彼女
 *  ITH hooked function: BGI#2 0x425550, called by 0x427450
 *
 *  00427450  /$ 6a ff          push -0x1  ; jichi: function starts
 *  00427452  |. 68 78634900    push sekajyo_.00496378                   ;  se handler installation
 *  00427457  |. 64:a1 00000000 mov eax,dword ptr fs:[0]
 *  0042745d  |. 50             push eax
 *  0042745e  |. 64:8925 000000>mov dword ptr fs:[0],esp
 *  00427465  |. 81ec d80c0000  sub esp,0xcd8
 *  0042746b  |. 8b8424 080d000>mov eax,dword ptr ss:[esp+0xd08]
 *  00427472  |. 56             push esi
 *  00427473  |. 8d8c24 3801000>lea ecx,dword ptr ss:[esp+0x138]
 *  0042747a  |. 50             push eax
 *  0042747b  |. 51             push ecx
 *  0042747c  |. 8b0d e0464b00  mov ecx,dword ptr ds:[0x4b46e0]
 *  00427482  |. e8 f9fdfdff    call sekajyo_.00407280
 *  00427487  |. 33f6           xor esi,esi
 *  00427489  |. 898424 b800000>mov dword ptr ss:[esp+0xb8],eax
 *  00427490  |. 3bc6           cmp eax,esi
 *  00427492  |. 0f84 95140000  je sekajyo_.0042892d
 *  00427498  |. 53             push ebx
 *  00427499  |. 55             push ebp
 *  0042749a  |. 8bac24 fc0c000>mov ebp,dword ptr ss:[esp+0xcfc]
 *  004274a1  |. 57             push edi
 *  004274a2  |. 89b424 b400000>mov dword ptr ss:[esp+0xb4],esi
 *  004274a9  |. 897424 10      mov dword ptr ss:[esp+0x10],esi
 *  004274ad  |. 8a45 00        mov al,byte ptr ss:[ebp]
 *  004274b0  |. b9 01000000    mov ecx,0x1
 *  004274b5  |. 3c 20          cmp al,0x20
 *  004274b7  |. 7d 68          jge short sekajyo_.00427521
 *  004274b9  |. 0fbec0         movsx eax,al
 *  004274bc  |. 83c0 fe        add eax,-0x2                             ;  switch (cases 2..8)
 *
 *  Sample game: FORTUNE ARTERIAL
 *  ITH hooked function: BGI#2 sub_41EBD0, called by 0x4207e0
 *
 *  0041ebcd     90             nop
 *  0041ebce     90             nop
 *  0041ebcf     90             nop
 *  004207e0  /$ 81ec 30090000  sub esp,0x930   ; jichi: function starts
 *  004207e6  |. 8b8424 5409000>mov eax,dword ptr ss:[esp+0x954]
 *  004207ed  |. 56             push esi
 *  004207ee  |. 8d8c24 0401000>lea ecx,dword ptr ss:[esp+0x104]
 *  004207f5  |. 50             push eax
 *  004207f6  |. 51             push ecx
 *  004207f7  |. 8b0d 48634900  mov ecx,dword ptr ds:[0x496348]
 *  004207fd  |. e8 ee47feff    call bgi.00404ff0
 *  00420802  |. 33f6           xor esi,esi
 *  00420804  |. 894424 54      mov dword ptr ss:[esp+0x54],eax
 *  00420808  |. 3bc6           cmp eax,esi
 *  0042080a  |. 0f84 94080000  je bgi.004210a4
 *  00420810  |. 53             push ebx
 *  00420811  |. 55             push ebp
 *  00420812  |. 8bac24 4809000>mov ebp,dword ptr ss:[esp+0x948]
 *  00420819  |. 57             push edi
 *  0042081a  |. 897424 54      mov dword ptr ss:[esp+0x54],esi
 *  0042081e  |. 897424 10      mov dword ptr ss:[esp+0x10],esi
 *  00420822  |. 8a45 00        mov al,byte ptr ss:[ebp]
 *  00420825  |. 3c 20          cmp al,0x20
 *  00420827  |. 7d 69          jge short bgi.00420892
 *  00420829  |. 0fbec0         movsx eax,al
 *  0042082c  |. 83c0 fe        add eax,-0x2                             ;  switch (cases 2..8)
 *  0042082f  |. 83f8 06        cmp eax,0x6
 *  00420832  |. 77 5e          ja short bgi.00420892
 *  00420834  |. ff2485 ac10420>jmp dword ptr ds:[eax*4+0x4210ac]
 *  0042083b  |> c74424 54 0100>mov dword ptr ss:[esp+0x54],0x1          ;  case 2 of switch 0042082c
 *  00420843  |. eb 45          jmp short bgi.0042088a
 *  00420845  |> 8d5424 1c      lea edx,dword ptr ss:[esp+0x1c]          ;  case 4 of switch 0042082c
 *  00420849  |. 68 0c424800    push bgi.0048420c
 *  0042084e  |. 52             push edx
 *  0042084f  |. eb 29          jmp short bgi.0042087a
 *  00420851  |> 68 08424800    push bgi.00484208                        ;  case 5 of switch 0042082c
 *  00420856  |. eb 1d          jmp short bgi.00420875
 *  00420858  |> 8d4c24 1c      lea ecx,dword ptr ss:[esp+0x1c]          ;  case 6 of switch 0042082c
 *  0042085c  |. 68 04424800    push bgi.00484204
 *  00420861  |. 51             push ecx
 *  00420862  |. eb 16          jmp short bgi.0042087a
 *  00420864  |> 8d5424 1c      lea edx,dword ptr ss:[esp+0x1c]          ;  case 7 of switch 0042082c
 *  00420868  |. 68 00424800    push bgi.00484200
 *  0042086d  |. 52             push edx
 *  0042086e  |. eb 0a          jmp short bgi.0042087a
 *  00420870  |> 68 fc414800    push bgi.004841fc                        ;  case 8 of switch 0042082c
 *  00420875  |> 8d4424 20      lea eax,dword ptr ss:[esp+0x20]
 *  00420879  |. 50             push eax
 *  0042087a  |> c74424 18 0100>mov dword ptr ss:[esp+0x18],0x1
 *  00420882  |. e8 b9a7ffff    call bgi.0041b040
 *  00420887  |. 83c4 08        add esp,0x8
 *  0042088a  |> 45             inc ebp
 *  0042088b  |. 89ac24 4c09000>mov dword ptr ss:[esp+0x94c],ebp
 *  00420892  |> 8b9c24 3001000>mov ebx,dword ptr ss:[esp+0x130]         ;  default case of switch 0042082c
 *  00420899  |. 8d8c24 1001000>lea ecx,dword ptr ss:[esp+0x110]
 *  004208a0  |. 51             push ecx
 *  004208a1  |. 895c24 70      mov dword ptr ss:[esp+0x70],ebx
 *  004208a5  |. e8 76080000    call bgi.00421120
 *  004208aa  |. 894424 34      mov dword ptr ss:[esp+0x34],eax
 *  004208ae  |. 8b8424 5409000>mov eax,dword ptr ss:[esp+0x954]
 *  004208b5  |. 83c4 04        add esp,0x4
 *  004208b8  |. 3bc6           cmp eax,esi
 *  004208ba  |. 74 0f          je short bgi.004208cb
 *  004208bc  |. 53             push ebx
 *  004208bd  |. e8 7e080000    call bgi.00421140
 */
static ulong searchBGI1(ulong startAddress, ulong stopAddress)
{
  //return 0x4207e0; // FORTUNE ARTERIAL
  //const BYTE ins[] = {
  //  0x8a,0x45, 0x00,  // 00420822  |. 8a45 00        mov al,byte ptr ss:[ebp]
  //  0x3c, 0x20,       // 00420825  |. 3c 20          cmp al,0x20
  //  0x7d, 0x69,       // 00420827  |. 7d 69          jge short bgi.00420892
  //  0x0f,0xbe,0xc0,   // 00420829  |. 0fbec0         movsx eax,al
  //  0x83,0xc0, 0xfe,  // 0042082c  |. 83c0 fe        add eax,-0x2                             ;  switch (cases 2..8)
  //  0x83,0xf8, 0x06,  // 0042082f  |. 83f8 06        cmp eax,0x6
  //  0x77, 0x5e        // 00420832  |. 77 5e          ja short bgi.00420892
  //};
  //enum { hook_offset = 0x4207e0 - 0x420822 }; // distance to the beginning of the function

  const BYTE ins[] = {
     0x0f,0xaf,0xcb,   // 004208de  |. 0fafcb         imul ecx,ebx
     0xf7,0xe9,        // 004208e1  |. f7e9           imul ecx
     0xc1,0xfa, 0x05,  // 004208e3  |. c1fa 05        sar edx,0x5
     0x8b,0xc2,        // 004208e6  |. 8bc2           mov eax,edx
     0xc1,0xe8, 0x1f,  // 004208e8  |. c1e8 1f        shr eax,0x1f
     0x03,0xd0,        // 004208eb  |. 03d0           add edx,eax
     0x8b,0xfa,        // 004208ed  |. 8bfa           mov edi,edx
     0x85,0xff,        // 004208ef  |. 85ff           test edi,edi
  };
  //enum { hook_offset = 0x4207e0 - 0x4208de }; // distance to the beginning of the function
  ulong range = min(stopAddress - startAddress, Engine::MaximumMemoryRange);
  ulong reladdr = MemDbg::searchPattern(startAddress, range, ins, sizeof(ins));
  if (!reladdr)
    //ConsoleOutput("vnreng:BGI2: pattern not found");
    return 0;

  //ulong addr = startAddress + reladdr + hook_offset;
  ulong addr = startAddress + reladdr;
  enum : WORD {
    sub_esp = 0xec81   // 004207e0  /$ 81ec 30090000
    , push_ff = 0xff6a // 00427450  /$ 6a ff   push -0x1, seh handler
  };
  for (int i = 0; i < 300; i++, addr--)
    if (*(WORD *)addr == sub_esp) { // beginning of the function without seh

      // Sample game: 世界征服彼女 with SEH
      // 00427450  /$ 6a ff          push -0x1
      // 00427452  |. 68 78634900    push sekajyo_.00496378                   ;  se handler installation
      // 00427457  |. 64:a1 00000000 mov eax,dword ptr fs:[0]
      // 0042745d  |. 50             push eax
      // 0042745e  |. 64:8925 000000>mov dword ptr fs:[0],esp
      // 00427465  |. 81ec d80c0000  sub esp,0xcd8
      //
      // 0x00427465 - 0x00427450 == 21
      ulong seh_addr = addr;
      for (int j = 0; j < 40; j++, seh_addr--)
        if (*(WORD *)seh_addr == push_ff) // beginning of the function with seh
          return seh_addr;
      return addr;
    }

  return 0;
}

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
  //return startAddress + 0x31850; // 世界と世界の真ん中 体験版
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

// BGI2 pattern also exists in BGI1
ulong BGIEngine::search(ulong startAddress, ulong stopAddress)
{
  ulong addr = ::searchBGI2(startAddress, stopAddress);
  if (!addr)
    addr = ::searchBGI1(startAddress, stopAddress);
  return addr;
}

// EOF
