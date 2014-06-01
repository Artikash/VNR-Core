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

//extern "C" { //
namespace { // unnamed

/**
 *  Sample game: 聖娼女 体験版
 *
 *  IDA:  sub_42F9B0 proc near ; Attributes: bp-based frame
 *  - var_8= dword ptr -8
 *  - var_4= byte ptr -4
 *  - var_3= word ptr -3
 *  - arg_0= dword ptr  8
 *  - arg_4= dword ptr  0Ch
 *  - arg_8= dword ptr  10h
 *
 *  - arg1: pointer
 *  - arg2: pointer, arg2+0xc is the text address
 *    - 0x0: 0
 *    - 0x4: int
 *    - 0x8: LPCWSTR
 *    - 0xc: LPCWSTR
 *  - arg3: int
 *  - return:  the same as [arg2 + 0xc]
 *
 *  Sample stack o arg2:
 *  002af6d8  00 00 00 00 03 02 00 00 ; arg2
 *  002af6e0  78 90 2d 01 60 9e 99 07 ; arg2 + 8
 *  002af6e8  83 68 82 c9 93 7c 82 ea
 *  002af6f0  82 b1 82 00 72 00 00 00
 *  002af6f8  9d 00 00 00 00 00 00 00
 */
typedef LPCSTR (__cdecl *hook_fun_t)(DWORD, DWORD, DWORD);
hook_fun_t oldHookFun;

__declspec(noinline)
static LPCSTR dispatchText(LPCSTR text, DWORD returnAddress, DWORD split);

/**
 *  Hooking thiscall using fastcall: http://tresp4sser.wordpress.com/2012/10/06/how-to-hook-thiscall-functions/
 *  - thiscall: this is in ecx and the first argument
 *  - fastcall: the first two parameters map to ecx and edx
 */
LPCSTR __cdecl newHookFun(DWORD arg1, DWORD arg2, DWORD arg3)
{
  enum { role = Engine::ScenarioRole, signature = role }; // dummy non-zero signature
  //return oldHookFun(self, arg1, arg2);
#ifdef DEBUG
  qDebug() << (*(LPCSTR*)(arg2 + 0xc)) << ":"
           << " signature: " << QString::number(signature, 16);
#endif // DEBUG
  //LPCSTR text = *(LPCSTR *)(arg2 + 0xc);
  //dispatchText(text, 0, 0);
  qDebug()<< 111111111;
  qDebug()<< 222222222;
  qDebug()<< 333333333;
  LPCSTR ret = oldHookFun(arg1, arg2, arg3);
  qDebug()<< 444444444;
  qDebug()<< 555555555;
  return ret;

  //QString text = QString::fromWCharArray(self->text(), self->size);
  //text = q->dispatchTextW(text, signature, role);
  //if (text.isEmpty())
  //  return self->size * 2; // estimated painted bytes

  //auto oldText0 = self->texts[0];
  //auto oldSize = self->size;
  //auto oldCapacity = self->capacity;
  //self->texts[0] = (LPCWSTR)text.utf16(); // lack trailing null character
  //self->size = text.size();
  //self->capacity = qMax(8, text.size()); // prevent using smaller size

  //int ret = oldHookFun(self, arg1, arg2); // ret = size * 2

  //self->texts[0] = oldText0;
  //self->size = oldSize;
  //self->capacity = oldCapacity;
  //return ret;
}

} // extern "C"

extern "C" static LPCSTR dispatchText(LPCSTR text, DWORD returnAddress, DWORD split)
{
  return text;
  //static QByteArray ret; // persistent storage, which makes this function not thread-safe

  // FIXME: It is ridiculous that accessing split will break extern "C"
  //auto sig = Engine::hashThreadSignature(returnAddress, split);
  //auto sig = Engine::hashThreadSignature(returnAddress);
  //Q_UNUSED(split);

  // split: choices: 0, character name & scenario: 1
  //auto role = split ? Engine::UnknownRole : Engine::ChoiceRole;
  enum { sig = 1, role = 1 };
  //ret = AbstractEngine::instance()->dispatchTextA(text, sig, role);
  //return (LPCSTR)ret.constData();
}

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
  //const BYTE ins[] = { // size = 14
  //  0x01,0x53, 0x58,                // 0153 58          add dword ptr ds:[ebx+58],edx
  //  0x8b,0x95, 0x34,0xfd,0xff,0xff, // 8b95 34fdffff    mov edx,dword ptr ss:[ebp-2cc]
  //  0x8b,0x43, 0x58,                // 8b43 58          mov eax,dword ptr ds:[ebx+58]
  //  0x3b,0xd7                       // 3bd7             cmp edx,edi ; hook here
  //};
  //enum { cur_ins_size = 2 };
  //enum { hook_offset = sizeof(ins) - cur_ins_size }; // = 14 - 2  = 12, current inst is the last one
  const BYTE ins1[] = {
    0x3b,0xd7, // 013baf32  |. 3bd7       |cmp edx,edi ; jichi: ITH hook here, char saved in edi
    0x75,0x4b  // 013baf34  |. 75 4b      |jnz short siglusen.013baf81
  };
  //enum { hook_offset = 0 };
  DWORD range1 = min(stopAddress - startAddress, Engine::MaximumMemoryRange);
  DWORD reladdr = MemDbg::searchPattern(startAddress, range1, ins1, sizeof(ins1));

  if (!reladdr)
    //ConsoleOutput("vnreng:Silkys2: pattern not found");
    return 0;

  const BYTE ins2[] = {
    0x55,      // 013bac70  /$ 55       push ebp ; jichi: function starts
    0x8b,0xec, // 013bac71  |. 8bec     mov ebp,esp
    0x6a,0xff  // 013bac73  |. 6a ff    push -0x1
  };
  enum { range2 = 0x300 }; // 0x013baf32  -0x013bac70 = 706 = 0x2c2
  DWORD addr = startAddress + reladdr - range2;
  reladdr = MemDbg::searchPattern(addr, range2, ins2, sizeof(ins2));
  if (!reladdr)
    //ConsoleOutput("vnreng:Silkys2: pattern not found");
    return 0;
  addr += reladdr;
  return addr;
}

bool SilkysEngine::attach()
{
  DWORD startAddress,
        stopAddress;
  if (!Engine::getCurrentMemoryRange(&startAddress, &stopAddress))
    return false;
  //ulong addr = ::searchSilkys(startAddress, stopAddress);
  DWORD addr = startAddress + 0x2f9b0; // 愛姉妹4
  dmsg(*(BYTE *)addr); // supposed to be 0x55
  //dmsg(addr - startAddress);
  if (!addr)
    return false;
  return ::oldHookFun = detours::replace<hook_fun_t>(addr, ::newHookFun);
}

// EOF
