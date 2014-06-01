// silkys.cc
// 5/31/2014 jichi
// About MES and Silky's arc
// See: http://www.dsl.gr.jp/~sage/mamitool/case/elf/aishimai.html
// MES: http://tieba.baidu.com/p/2068362185
#include "engine/model/silkys.h"
#include "engine/enginedef.h"
#include "engine/engineenv.h"
#include "engine/enginehash.h"
#include "detoursutil/detoursutil.h"
#include "memdbg/memsearch.h"
#include <QtCore/QStringList>
#include <qt_windows.h>

#include "debug.h"
#include <QDebug>
//#define DEBUG "siglus"
#include "sakurakit/skdebug.h"

// Used to get function's return address
// http://stackoverflow.com/questions/8797943/finding-a-functions-address-in-c
//#pragma intrinsic(_ReturnAddress)

/** Private data */

namespace { // unnamed

/**
 *  Type1: SEXティーチャー剛史 trial, reladdr = 0x2f0f0, 2 parameters
 *  Type2: 愛姉妹4, reladdr = 0x2f9b0, 3 parameters
 *
 *  FIXME: No idea why it crashes the game
 */
typedef LPCSTR (__cdecl *hook_fun_t)(DWORD);//, DWORD);
hook_fun_t oldHookFun;

/**
 *  Observations
 *  - Return: text
 */
LPCSTR __cdecl newHookFun(DWORD arg1) //, DWORD arg2)
{
  DWORD arg2_scene = arg1 + 4*5,
        arg2_chara = arg1 + 4*10;
  int role;
  LPCSTR text = nullptr;
  if (*(DWORD *)arg2_scene == 0) {
    text = *(LPCSTR *)(arg2_scene + 0xc);
    role = Engine::ScenarioRole;
  } else if (*(DWORD *)arg2_chara == 0) {
    text = LPCSTR(arg2_chara + 0xc);
    role = Engine::NameRole;
  }
  if (text) {
    QByteArray data = AbstractEngine::instance()->dispatchTextA(text, role, role);
    if (data.isEmpty())
      return text;
    if (role == Engine::ScenarioRole) {
      LPCSTR *p = (LPCSTR *)(arg2_scene + 0xc);
      *p = data.constData();
      oldHookFun(arg1);
      *p = text;
    } else
      oldHookFun(arg1);
    return text;
  }
  LPCSTR ret = oldHookFun(arg1);//, arg2);
  return ret;
}

} // unnamed namespace

/** Public class */

bool SilkysEngine::match()
{
  //return Engine::exists("Silkys.exe"); // It might not might not have this file
  // All arc files in 愛姉妹4 are: data, effect, layer, mes, music
  // mes.arc is the scenario
  return Engine::exists(QStringList() << "data.arc" << "effect.arc" << "mes.arc");
}

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
 */
static DWORD searchSilkys(DWORD startAddress, DWORD stopAddress)
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
  DWORD range = min(stopAddress - startAddress, Engine::MaximumMemoryRange);
  DWORD reladdr = MemDbg::searchPattern(startAddress, range, ins, sizeof(ins));
  //ITH_GROWL_DWORD(reladdr);
  //reladdr = 0x2f9b0; // 愛姉妹4
  //reladdr = 0x2f0f0; // SEXティーチャー剛史 trial
  if (!reladdr)
    return 0;

  DWORD addr = startAddress + reladdr;
  enum : BYTE { push_ebp = 0x55 };
  for (int i = 0; i < 0x20; i++, addr--) // value of i is supposed to be 0xc or 0x10
    if (*(BYTE *)addr == push_ebp) // beginning of the function
      return addr;
  return 0;
}

bool SilkysEngine::attach()
{
  DWORD startAddress,
        stopAddress;
  if (!Engine::getCurrentMemoryRange(&startAddress, &stopAddress))
    return false;
  ulong addr = ::searchSilkys(startAddress, stopAddress);
  //DWORD addr = startAddress + 0x2f9b0; // 愛姉妹4
  //DWORD addr = startAddress + 0x2f0f0; // SEXティーチャー剛史 trial
  dmsg(*(BYTE *)addr); // supposed to be 0x55
  //dmsg(addr - startAddress);
  if (!addr)
    return false;
  return ::oldHookFun = detours::replace<hook_fun_t>(addr, ::newHookFun);
}

// EOF
