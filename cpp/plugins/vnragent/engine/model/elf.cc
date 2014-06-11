// elf.cc
// 5/31/2014 jichi
// About MES and Silky's arc
// See: http://www.dsl.gr.jp/~sage/mamitool/case/elf/aishimai.html
// MES: http://tieba.baidu.com/p/2068362185
#include "engine/model/elf.h"
#include "engine/enginedef.h"
#include "engine/engineenv.h"
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
  Q_UNUSED(edx);
  ulong arg2_scene = arg1 + 4*5,
        arg2_chara = arg1 + 4*10;
  auto q = AbstractEngine::instance();

  // Scenario
  if (*(DWORD *)arg2_scene == 0) {
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
  } else if (*(DWORD *)arg2_chara == 0) {
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
}

} // unnamed namespace

/**
 *  jichi 8/16/2013: Insert new siglus hook
 *  See (CaoNiMaGeBi): http://tieba.baidu.com/p/2531786952
 *
 *  013bac6e     cc             int3
 *  013bac6f     cc             int3
 *  013bac70  /$ 55             push ebp ; jichi: function starts
 *  013bac71  |. 8bec           mov ebp,esp
 *  013bac73  |. 6a ff          push -0x1
 *  013bac75  |. 68 d8306201    push siglusen.016230d8
 *  013bac7a  |. 64:a1 00000000 mov eax,dword ptr fs:[0]
 *  013bac80  |. 50             push eax
 *  013bac81  |. 81ec dc020000  sub esp,0x2dc
 *  013bac87  |. a1 90f46b01    mov eax,dword ptr ds:[0x16bf490]
 *  013bac8c  |. 33c5           xor eax,ebp
 *  013bac8e  |. 8945 f0        mov dword ptr ss:[ebp-0x10],eax
 *  013bac91  |. 53             push ebx
 *  013bac92  |. 56             push esi
 *  013bac93  |. 57             push edi
 *  013bac94  |. 50             push eax
 *  ...
 *  013baf32  |. 3bd7           |cmp edx,edi ; jichi: ITH hook here, char saved in edi
 *  013baf34  |. 75 4b          |jnz short siglusen.013baf81
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
  return ::oldHookFun = replaceFunction<hook_fun_t>(addr, ::newHookFun);
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
