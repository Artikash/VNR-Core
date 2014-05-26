// siglus.cc
// 5/25/2014 jichi
//
// Hooking thiscall: http://tresp4sser.wordpress.com/2012/10/06/how-to-hook-thiscall-functions/
#include "engine/model/siglus.h"
#include "engine/enginedef.h"
#include "engine/engineenv.h"
#include "engine/enginehash.h"
#include "detoursutil/detoursutil.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>

#define DEBUG "siglus"
#include "sakurakit/skdebug.h"

// Used to get function's return address
// http://stackoverflow.com/questions/8797943/finding-a-functions-address-in-c
//#pragma intrinsic(_ReturnAddress)

/** Private data */

namespace { // unnamed

/**
 *  Sample game: 聖娼女 体験版
 *
 *  IDA: sub_4DAC70 proc near ; Attributes: bp-based frame
 *
 *  Observations:
 *  - return: number of bytes = 2 * number of size
 *  - arg1: unknown pointer, remains the same
 *  - arg2: unknown, remains the same
 *  - this (ecx)
 *    - union
 *      - char x 3: if size < (3 * 2 - 1) &&
 *      - pointer x 4
 *        - 0x0: UTF-16 text
 *        - 0x4: the same as 0x0
 *        - 0x8: unknown variate pointer
 *    - 0xc: wchar_t pointer to a flag, the pointed value is zero when union is used as a char
 *    - 0x10: size of the text without null char
 *    - 0x14: unknown size, always slightly larger than size
 *    - 0x18: constant pointer
 *    ...
 */
typedef int (__thiscall *hook_fun_t)(void *, DWORD, DWORD);
// Use __fastcall to completely forward ecx and edx
//typedef int (__fastcall *hook_fun_t)(void *, void *, DWORD, DWORD);
hook_fun_t oldHookFun;

struct HookStruct
{
  union {
    LPCWSTR texts[3]; // 0x0
    WCHAR text[6];  // 0x0
  };
  LPWORD flag;      // 0xc
  DWORD size;       // 0x10
  //DWORD capacity; // 0x14
};

/**
 *  Hooking thiscall using fastcall: http://tresp4sser.wordpress.com/2012/10/06/how-to-hook-thiscall-functions/
 *  - thiscall: this is in ecx and the first argument
 *  - fastcall: the first two parameters map to ecx and edx
 */
int __fastcall newHookFun(HookStruct *self, void *edx, DWORD arg1, DWORD arg2)
{
  Q_UNUSED(edx);
  enum { role = Engine::ScenarioRole, signature = 1 }; // dummy signature
  auto q = AbstractEngine::instance();

  if (self->flag && !*self->flag) {
#ifdef DEBUG
    qDebug() << QString::fromWCharArray(self->text);
#endif // DEBUG
    QString text = QString::fromWCharArray(self->text, self->size);
    text = q->dispatchTextW(text, signature, role);
    if (text.isEmpty())
      return 0;

    // FIXME: replacement is not implemented for short text
    return oldHookFun(self, arg1, arg2);
  }

  //return oldHookFun(self, arg1, arg2);
#ifdef DEBUG
  if (self->flag && *self->flag)
    qDebug() << QString::fromWCharArray(self->texts[0]) << ":"
             << (self->text[0] == self->text[1]) << ":"
             << (DWORD)(*self->flag) << ":"
             << self->size << ";"
             << arg1 << ","
             << arg2 << ";"
             << " signature: " << QString::number(signature, 16);
#endif // DEBUG

  QString text = QString::fromWCharArray(self->texts[0], self->size);
  text = q->dispatchTextW(text, signature, role);
  if (text.isEmpty())
    return 0;
    //return text.size() * 2;

  auto oldSize = self->size;
  auto oldText = self->texts[0];
  self->size = text.size();
  self->texts[0] = (LPCWSTR)text.utf16();

  int ret = oldHookFun(self, arg1, arg2); // supposed to equal size * 2

  self->size = oldSize;
  self->texts[0] = oldText;
  return ret;
}

} // unnamed namespace

/** Public class */

bool SiglusEngine::match() { return Engine::exists("SiglusEngine.exe"); }

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
static DWORD searchSiglus2(DWORD startAddress, DWORD stopAddress)
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
    //ConsoleOutput("vnreng:Siglus2: pattern not found");
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
    //ConsoleOutput("vnreng:Siglus2: pattern not found");
    return 0;
  addr += reladdr;
  return addr;
}

bool SiglusEngine::attach()
{
  DWORD startAddress,
        stopAddress;
  if (!Engine::getCurrentMemoryRange(&startAddress, &stopAddress))
    return false;
  ulong addr = ::searchSiglus2(startAddress, stopAddress);
  //DWORD addr = startAddress + 0xdb140; // 聖娼女
  //dmsg(addr - startAddress);
  if (!addr)
    return false;
  return ::oldHookFun = detours::replace<hook_fun_t>(addr, ::newHookFun);
}

// EOF
