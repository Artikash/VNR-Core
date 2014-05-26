// bgi1.cc
// 5/11/2014 jichi
#include "engine/model/bgi.h"
#include "engine/enginedef.h"
#include "engine/engineenv.h"
#include "engine/enginehash.h"
#include "detoursutil/detoursutil.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>

#pragma intrinsic(_ReturnAddress)

//#define DEBUG "bgi1"

#include "sakurakit/skdebug.h"
#ifdef DEBUG
# include "debug.h"
#endif // DEBUG

/** Private data */

/**
 *  FORTUNE ARTERIAL, 0x4207E0
 *
 *  TODO: Figure out the meaning of the return value
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
 */
extern "C" { // C linkage is indispensable for BGI engine

typedef int (__cdecl *hook_fun_t)(DWORD, DWORD, LPCSTR, DWORD, DWORD, DWORD,
                                  DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD);
/**
 *  Declaring this variable as static will break the extern "C" declaration.
 *  Calling it will have no effect in BGI game.
 */
/*static*/ hook_fun_t BGI1_oldHookFun;

__declspec(noinline)
static LPCSTR dispatchText(LPCSTR text, DWORD returnAddress, DWORD split);

static int __cdecl newHookFun(DWORD arg1, DWORD arg2, LPCSTR str, DWORD arg4, DWORD arg5, DWORD arg6,
                              DWORD arg7, DWORD split, DWORD arg9, DWORD arg10, DWORD arg11, DWORD arg12, DWORD arg13)

{
#ifdef DEBUG
  qDebug() << arg1 << ":"
           << arg2 << ":"
           << QString::fromLocal8Bit(str) << ":"
           << split;
#endif // DEBUG
  if (str) // str could be nullptr
    str = dispatchText(str, (DWORD)_ReturnAddress(), split);
  return !str ? 0 : // TODO: investigate the meaning of the return value
         BGI1_oldHookFun(arg1, arg2, str, arg4, arg5, arg6, arg7, split, arg9, arg10, arg11, arg12, arg13);
}

} // extern "C"

/**
 *  It is OK to declare a function as extern "C", but define it using C++ syntax.
 *  See: http://stackoverflow.com/questions/7281441/elegantly-call-c-from-c
 */
/*extern "C"*/ static LPCSTR dispatchText(LPCSTR text, DWORD returnAddress, DWORD split)
{
  static QByteArray ret; // persistent storage, which makes this function not thread-safe
  auto sig = Engine::hashThreadSignature(returnAddress, split);
  //ret = AbstractEngine::instance()->dispatchTextA(text, sig, Engine::ScenarioRole);
  ret = AbstractEngine::instance()->dispatchTextA(text, sig);
  return (LPCSTR)ret.constData();
}

/**
 *  5/12/2014
 *  This is the caller of the ITH BGI hook, which extract text by characters
 *  and cannot be used for substition.
 *
 *  Sample game: FORTUNE ARTERIAL
 *
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
static DWORD searchBGI1(DWORD startAddress, DWORD stopAddress)
{
  // TODO: This function is incomplete
  const BYTE ins[] = {
    0x8a,0x45, 0x00,  // 00420822  |. 8a45 00        mov al,byte ptr ss:[ebp]
    0x3c, 0x20,       // 00420825  |. 3c 20          cmp al,0x20
    0x7d, 0x69,       // 00420827  |. 7d 69          jge short bgi.00420892
    0x0f,0xbe,0xc0,   // 00420829  |. 0fbec0         movsx eax,al
    0x83,0xc0, 0xfe,  // 0042082c  |. 83c0 fe        add eax,-0x2                             ;  switch (cases 2..8)
    0x83,0xf8, 0x06,  // 0042082f  |. 83f8 06        cmp eax,0x6
    0x77, 0x5e        // 00420832  |. 77 5e          ja short bgi.00420892
  };
  enum { hook_offset = 0x4207e0 - 0x420822 }; // distance to the beginning of the function
  DWORD range = min(stopAddress - startAddress, Engine::MaximumMemoryRange);
  DWORD reladdr = MemDbg::searchPattern(startAddress, range, ins, sizeof(ins));
  if (!reladdr)
    //ConsoleOutput("vnreng:BGI2: pattern not found");
    return 0;

  DWORD addr = startAddress + reladdr + hook_offset;
  enum : WORD { sub_esp = 0xec81 };  // 004207e0  /$ 81ec 30090000
  if (*(WORD *)addr != sub_esp)
    //ConsoleOutput("vnreng:BGI2: pattern found but the function offset is invalid");
    return 0;

  return addr;
}

bool BGIEngine::attachBGIType1()
{
  ulong startAddress,
        stopAddress;
  if (!Engine::getCurrentMemoryRange(&startAddress, &stopAddress))
    return false;
  ulong addr = ::searchBGI1(startAddress, stopAddress);
  //ulong addr = 0x4207e0; // FORTUNE ARTERIAL
  if (!addr)
    return false;
  return ::BGI1_oldHookFun = detours::replace<hook_fun_t>(addr, newHookFun);
}

// EOF