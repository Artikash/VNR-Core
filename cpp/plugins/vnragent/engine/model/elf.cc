// elf.cc
// 5/31/2014 jichi
// About MES and Silky's arc
// See: http://www.dsl.gr.jp/~sage/mamitool/case/elf/aishimai.html
// MES: http://tieba.baidu.com/p/2068362185
#include "engine/model/elf.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/engineutil.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>
#include <QtCore/QStringList>

namespace { // unnamed

typedef LPCSTR (__thiscall *hook_fun_t)(void *, DWORD, DWORD, DWORD);
// Use __fastcall to completely forward ecx and edx
//typedef int (__fastcall *hook_fun_t)(void *, void *, DWORD, DWORD);
hook_fun_t oldHookFun;

/**
 *  Type1: SEXティーチャー剛史 trial, reladdr = 0x2f0f0, 2 parameters
 *  Type2: 愛姉妹4, reladdr = 0x2f9b0, 3 parameters
 *
 *  Observations
 *  - Return: text
 *  - Text is calculated from arg1
 */
LPCSTR __fastcall newHookFun(void *self, void *edx, DWORD arg1, DWORD arg2, DWORD arg3)
{
#define deref(x) *(DWORD *)(x)
  Q_UNUSED(edx);
  ulong arg2_scene = arg1 + 4*5,
        arg2_chara = arg1 + 4*10;
  auto q = EngineController::instance();

  // Scenario
  if (deref(arg2_scene) == 0) {
    enum { role = Engine::ScenarioRole, signature = Engine::ScenarioThreadSignature };

    // By debugging, this text is later released using heapFree
    LPCSTR *p = (LPCSTR *)(arg2_scene + 0xc),
           text = *p;
    QByteArray data = q->dispatchTextA(text, signature, role);
    *p = (LPCSTR)data.constData();
    LPCSTR ret = oldHookFun(self, arg1, arg2, arg3);
    *p = text; // restore old hook
    return ret;

  // Name
  // FIXME: The name has to be truncated
  } else if (deref(arg2_chara) == 0) {
    enum { role = Engine::NameRole, signature = Engine::NameThreadSignature };

    LPSTR srcText = LPSTR(arg2_chara + 0xc);
    QByteArray srcData = srcText,
               replData = q->dispatchTextA(srcData, signature, role);
    if (!replData.isEmpty())
      ::memcpy(srcText, replData.constData(), min(srcData.size(), replData.size()));
    int left = srcData.size() - replData.size();
    if (left > 0)
      ::memset(srcText + srcData.size() - left, 0, left);
    return oldHookFun(self, arg1, arg2, arg3);

  // Warning: unknown game parameter
  } else
    return oldHookFun(self, arg1, arg2, arg3);
#undef deref
}

} // unnamed namespace

/**
 *  jichi 5/31/2014: elf's
 *  Type1: SEXティーチャー剛史 trial, reladdr = 0x2f0f0, 2 parameters
 *  Type2: 愛姉妹4, reladdr = 0x2f9b0, 3 parameters
 *
 *  IDA: sub_42F9B0 proc near ; bp-based frame
 *    var_8 = dword ptr -8
 *    var_4 = byte ptr -4
 *    var_3 = word ptr -3
 *    arg_0 = dword ptr  8
 *    arg_4 = dword ptr  0Ch
 *    arg_8 = dword ptr  10h
 *
 *  Call graph (Type2):
 *  0x2f9b0 ;  hook here
 *  > 0x666a0 ; called multiple time
 *  > TextOutA ; there are two TextOutA, the second is the right one
 *
 *  Function starts (Type1), pattern offset: 0xc
 *  - 012ef0f0  /$ 55             push ebp ; jichi: hook
 *  - 012ef0f1  |. 8bec           mov ebp,esp
 *  - 012ef0f3  |. 83ec 10        sub esp,0x10
 *  - 012ef0f6  |. 837d 0c 00     cmp dword ptr ss:[ebp+0xc],0x0
 *  - 012ef0fa  |. 53             push ebx
 *  - 012ef0fb  |. 56             push esi
 *  - 012ef0fc  |. 75 0f          jnz short stt_tria.012ef10d ; jicchi: pattern starts
 *  - 012ef0fe  |. 8b45 08        mov eax,dword ptr ss:[ebp+0x8]
 *  - 012ef101  |. 8b48 04        mov ecx,dword ptr ds:[eax+0x4]
 *  - 012ef104  |. 8b91 90000000  mov edx,dword ptr ds:[ecx+0x90] ; jichi: pattern stops
 *  - 012ef10a  |. 8955 0c        mov dword ptr ss:[ebp+0xc],edx
 *  - 012ef10d  |> 8b4d 08        mov ecx,dword ptr ss:[ebp+0x8]
 *  - 012ef110  |. 8b51 04        mov edx,dword ptr ds:[ecx+0x4]
 *  - 012ef113  |. 33c0           xor eax,eax
 *  - 012ef115  |. c645 f8 00     mov byte ptr ss:[ebp-0x8],0x0
 *  - 012ef119  |. 66:8945 f9     mov word ptr ss:[ebp-0x7],ax
 *  - 012ef11d  |. 8b82 b0000000  mov eax,dword ptr ds:[edx+0xb0]
 *  - 012ef123  |. 8945 f4        mov dword ptr ss:[ebp-0xc],eax
 *  - 012ef126  |. 33db           xor ebx,ebx
 *  - 012ef128  |> 8b4f 20        /mov ecx,dword ptr ds:[edi+0x20]
 *  - 012ef12b  |. 83f9 10        |cmp ecx,0x10
 *
 *  Function starts (Type2), pattern offset: 0x10
 *  - 0093f9b0  /$ 55             push ebp  ; jichi: hook here
 *  - 0093f9b1  |. 8bec           mov ebp,esp
 *  - 0093f9b3  |. 83ec 08        sub esp,0x8
 *  - 0093f9b6  |. 837d 10 00     cmp dword ptr ss:[ebp+0x10],0x0
 *  - 0093f9ba  |. 53             push ebx
 *  - 0093f9bb  |. 8b5d 0c        mov ebx,dword ptr ss:[ebp+0xc]
 *  - 0093f9be  |. 56             push esi
 *  - 0093f9bf  |. 57             push edi
 *  - 0093f9c0  |. 75 0f          jnz short silkys.0093f9d1 ; jichi: pattern starts
 *  - 0093f9c2  |. 8b45 08        mov eax,dword ptr ss:[ebp+0x8]
 *  - 0093f9c5  |. 8b48 04        mov ecx,dword ptr ds:[eax+0x4]
 *  - 0093f9c8  |. 8b91 90000000  mov edx,dword ptr ds:[ecx+0x90] ; jichi: pattern stops
 *  - 0093f9ce  |. 8955 10        mov dword ptr ss:[ebp+0x10],edx
 *  - 0093f9d1  |> 33c0           xor eax,eax
 *  - 0093f9d3  |. c645 fc 00     mov byte ptr ss:[ebp-0x4],0x0
 *  - 0093f9d7  |. 66:8945 fd     mov word ptr ss:[ebp-0x3],ax
 *  - 0093f9db  |. 33ff           xor edi,edi
 *  - 0093f9dd  |> 8b53 20        /mov edx,dword ptr ds:[ebx+0x20]
 *  - 0093f9e0  |. 8d4b 0c        |lea ecx,dword ptr ds:[ebx+0xc]
 *  - 0093f9e3  |. 83fa 10        |cmp edx,0x10
 *
 *  @param  stackSize  number of bytes on the stack of the function, which is the parameter of sub esp
 */
static ulong searchElf(ulong startAddress, ulong stopAddress, int *stackSize)
{
  const BYTE ins[] = {
      //0x55,                             // 0093f9b0  /$ 55             push ebp  ; jichi: hook here
      //0x8b,0xec,                        // 0093f9b1  |. 8bec           mov ebp,esp
      //0x83,0xec, 0x08,                  // 0093f9b3  |. 83ec 08        sub esp,0x8
      //0x83,0x7d, 0x10, 0x00,            // 0093f9b6  |. 837d 10 00     cmp dword ptr ss:[ebp+0x10],0x0
      //0x53,                             // 0093f9ba  |. 53             push ebx
      //0x8b,0x5d, 0x0c,                  // 0093f9bb  |. 8b5d 0c        mov ebx,dword ptr ss:[ebp+0xc]
      //0x56,                             // 0093f9be  |. 56             push esi
      //0x57,                             // 0093f9bf  |. 57             push edi
      0x75, 0x0f,                       // 0093f9c0  |. 75 0f          jnz short silkys.0093f9d1
      0x8b,0x45, 0x08,                  // 0093f9c2  |. 8b45 08        mov eax,dword ptr ss:[ebp+0x8]
      0x8b,0x48, 0x04,                  // 0093f9c5  |. 8b48 04        mov ecx,dword ptr ds:[eax+0x4]
      0x8b,0x91, 0x90,0x00,0x00,0x00    // 0093f9c8  |. 8b91 90000000  mov edx,dword ptr ds:[ecx+0x90]
  };
  //enum { hook_offset = 0xc };
  ulong range = min(stopAddress - startAddress, Engine::MaximumMemoryRange);
  ulong reladdr = MemDbg::searchPattern(startAddress, range, ins, sizeof(ins));
  //ITH_GROWL_DWORD(reladdr);
  //reladdr = 0x2f9b0; // 愛姉妹4
  //reladdr = 0x2f0f0; // SEXティーチャー剛史 trial
  if (!reladdr)
    return 0;

  ulong addr = startAddress + reladdr;
  enum : BYTE { push_ebp = 0x55 };
  for (int i = 0; i < 0x20; i++, addr--) // value of i is supposed to be 0xc or 0x10
    if (*(BYTE *)addr == push_ebp) { // beginning of the function
      // 012df0f0  /$ 55             push ebp   ; funtion starts
      // 012df0f1  |. 8bec           mov ebp,esp
      // 012df0f3  |. 83ec 10        sub esp,0x10
      // 012df0f6  |. 837d 0c 00     cmp dword ptr ss:[ebp+0xc],0x0
      *stackSize = ((BYTE *)addr)[5];
      return addr;
    }
  return 0;
}

bool ElfEngine::attach()
{
  ulong startAddress,
        stopAddress;
  if (!Engine::getCurrentMemoryRange(&startAddress, &stopAddress))
    return false;
  int stackSize;
  ulong addr = ::searchElf(startAddress, stopAddress, &stackSize);
  //DWORD addr = startAddress + 0x2f9b0; // 愛姉妹4
  //DWORD addr = startAddress + 0x2f0f0; // SEXティーチャー剛史 trial
  //dmsg(*(BYTE *)addr); // supposed to be 0x55
  //dmsg(addr - startAddress);
  //dmsg(stackSize);
  // FIXME: It only supports 2 parameters. Dynamically determine parameter size
  if (!addr || stackSize != 0x8)
    return false;
  return ::oldHookFun = Engine::replaceFunction<hook_fun_t>(addr, ::newHookFun);
}

// EOF

/*
void ElfEngine::hookFunction(HookStack *stack)
{
  static QByteArray data_; // persistent storage, which makes this function not thread-safe

  ulong arg1 = stack->args[0];
  ulong arg2_scene = arg1 + 4*5,
        arg2_chara = arg1 + 4*10;

  if (*(DWORD *)arg2_scene == 0) {
    // By debugging, this text is later released using heapFree
    LPCSTR *p = (LPCSTR *)(arg2_scene + 0xc),
           text = *p;
    enum { role = Engine::ScenarioRole, signature = Engine::ScenarioThreadSignature };
    data_ = instance()->dispatchTextA(text, signature, role);
    *p = (LPCSTR)data_.constData();
  } else if (*(DWORD *)arg2_chara == 0) {
    LPCSTR text = LPCSTR(arg2_chara + 0xc);
    enum { role = Engine::NameRole, signature = Engine::NameThreadSignature };
    data_ = instance()->dispatchTextA(text, signature, role);
  }
}
*/
