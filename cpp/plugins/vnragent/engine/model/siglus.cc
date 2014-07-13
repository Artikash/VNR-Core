// siglus.cc
// 5/25/2014 jichi
//
// Hooking thiscall: http://tresp4sser.wordpress.com/2012/10/06/how-to-hook-thiscall-functions/
#include "engine/model/siglus.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>

//#define DEBUG "siglus"
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
 *
 *  Sample stack:
 *  0025edf0  a8 f3 13 0a a8 f3 13 0a  ｨ・.ｨ・.   ; jichi: ecx = 0025edf0
 *            LPCWSTR     LPCWSTR
 *  0025edf8  10 ee 25 00 d0 ee 37 01  ・.ﾐ・
 *            LPCWSTR     LPCWSTR
 *  0025ee00  13 00 00 00 17 00 00 00  ...…
 *            SIZE_T      SIZE_T
 *
 *  0025ee08  18 0c f6 09 27 00 00 00  .・'... ; jichi: following three lines are constants
 *  0025ee10  01 00 00 00 01 00 00 00  ......
 *  0025ee18  d2 d9 5d 9f 1c a2 e7 09  ﾒﾙ]・｢・
 *
 *  0025ee20  40 8c 10 07 00 00 00 00  @・....
 *  0025ee28  00 00 00 00 00 00 00 00  ........
 *  0025ee30  b8 ee ce 0c b8 ee ce 0c  ｸ﨩.ｸ﨩.
 *  0025ee38  b8 ee ce 0c 00 00 00 00  ｸ﨩.....
 *  0025ee40  00 00 00 00 01 00 00 00  .......
 *  0025ee48  00 00 00 00 00 00 00 00  ........
 *  0025ee50  00 00 00 00 00 00 00 00  ........
 *  0025ee58  00 00 00 00 00 00 00 00  ........
 *
 *  0025ee60  01 00 00 00 01 00 00 00  ......
 */
typedef int (__thiscall *hook_fun_t)(void *, DWORD, DWORD); // the first pointer is this
// Use __fastcall to completely forward ecx and edx
//typedef int (__fastcall *hook_fun_t)(void *, void *, DWORD, DWORD);
hook_fun_t oldHookFun;

struct HookStruct
{
  union {
    LPCWSTR texts[4]; // 0x0
    WCHAR data[8];    // 0x0
  };
  DWORD size,         // 0x10
        capacity;     // 0x14

  // According to the assembly code, this[0x14] should be larger than 8
  // 004DACFA   mov     edx, [edi+14h] ; sub_4DAC70+130j ...
  // 004DACFD   cmp     edx, 8
  //            jb      short loc_4DAD06
  LPCWSTR text() const
  { return capacity < 8 ? data : texts[0]; }
};

/**
 *  Hooking thiscall using fastcall: http://tresp4sser.wordpress.com/2012/10/06/how-to-hook-thiscall-functions/
 *  - thiscall: this is in ecx and the first argument
 *  - fastcall: the first two parameters map to ecx and edx
 */
int __fastcall newHookFun(HookStruct *self, void *edx, DWORD arg1, DWORD arg2)
{
  Q_UNUSED(edx);
  enum { role = Engine::ScenarioRole, signature = Engine::ScenarioThreadSignature };
  //return oldHookFun(self, arg1, arg2);
#ifdef DEBUG
  if (self->size < 8)
    qDebug() << QString::fromWCharArray(self->texts[0]) << ":"
             << (self->text[0] == self->text[1]) << ":"
             << (DWORD)(*self->flag) << ":"
             << self->size << ";"
             << arg1 << ","
             << arg2 << ";"
             << " signature: " << QString::number(signature, 16);
#endif // DEBUG
  auto q = EngineController::instance();

  QString text = QString::fromWCharArray(self->text(), self->size);
  text = q->dispatchTextW(text, signature, role);
  if (text.isEmpty())
    return self->size * 2; // estimated painted bytes

  auto oldText0 = self->texts[0];
  auto oldSize = self->size;
  auto oldCapacity = self->capacity;
  self->texts[0] = (LPCWSTR)text.utf16(); // lack trailing null character
  self->size = text.size();
  self->capacity = max(8, text.size()); // prevent using smaller size

  int ret = oldHookFun(self, arg1, arg2); // ret = size * 2

  // Restoring is indispensible, and as a result, the default hook does not work
  self->texts[0] = oldText0;
  self->size = oldSize;
  self->capacity = oldCapacity;
  return ret;
}

} // unnamed namespace

/** Public class */

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
ulong SiglusEngine::search(ulong startAddress, ulong stopAddress)
{
  //const BYTE bytes[] = { // size = 14
  //  0x01,0x53, 0x58,                // 0153 58          add dword ptr ds:[ebx+58],edx
  //  0x8b,0x95, 0x34,0xfd,0xff,0xff, // 8b95 34fdffff    mov edx,dword ptr ss:[ebp-2cc]
  //  0x8b,0x43, 0x58,                // 8b43 58          mov eax,dword ptr ds:[ebx+58]
  //  0x3b,0xd7                       // 3bd7             cmp edx,edi ; hook here
  //};
  //enum { cur_ins_size = 2 };
  //enum { hook_offset = sizeof(bytes) - cur_ins_size }; // = 14 - 2  = 12, current inst is the last one
  const BYTE bytes1[] = {
    0x3b,0xd7, // 013baf32  |. 3bd7       |cmp edx,edi ; jichi: ITH hook here, char saved in edi
    0x75,0x4b  // 013baf34  |. 75 4b      |jnz short siglusen.013baf81
  };
  //enum { hook_offset = 0 };
  //ulong range1 = min(stopAddress - startAddress, Engine::MaximumMemoryRange);
  ulong addr = MemDbg::findBytes(bytes1, sizeof(bytes1), startAddress, stopAddress);
  if (!addr)
    //ConsoleOutput("vnreng:Siglus2: pattern not found");
    return 0;

  const BYTE bytes2[] = {
    0x55,      // 013bac70  /$ 55       push ebp ; jichi: function starts
    0x8b,0xec, // 013bac71  |. 8bec     mov ebp,esp
    0x6a,0xff  // 013bac73  |. 6a ff    push -0x1
  };
  enum { range = 0x300 }; // 0x013baf32  -0x013bac70 = 706 = 0x2c2
  return MemDbg::findBytes(bytes2, sizeof(bytes2), addr - range, addr);
  //if (!reladdr)
  //  //ConsoleOutput("vnreng:Siglus2: pattern not found");
  //  return 0;
  //addr += reladdr;
  //return addr;
}

bool SiglusEngine::attach()
{
  ulong startAddress,
        stopAddress;
  if (!Engine::getCurrentMemoryRange(&startAddress, &stopAddress))
    return false;
  ulong addr = search(startAddress, stopAddress);
  //ulong addr = startAddress + 0xdb140; // 聖娼女
  //ulong addr = startAddress + 0xdaf32; // 聖娼女 体験版
  //dmsg(addr - startAddress);
  if (!addr)
    return false;
  return ::oldHookFun = Engine::replaceFunction<hook_fun_t>(addr, ::newHookFun);
}

// EOF
