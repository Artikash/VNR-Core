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
 *  - this (ecx)
 *    - 0x0: UTF-16 text
 *    - 0x4: the same as 0x0
 *    - 0x8: unknown variate pointer
 *    - 0xc: unknown variate pointer, seems to be a function address
 *    - 0x10: size of the text
 *    - 0x14: unknown size
 *    - 0x18: constant pointer
 *    ...
 */
typedef int (__thiscall *hook_fun_t)(void *, DWORD, DWORD);
// Use __fastcall to completely forward ecx and edx
//typedef int (__fastcall *hook_fun_t)(void *, void *, DWORD, DWORD);
hook_fun_t oldHookFun;

struct HookStruct
{
  LPCWSTR text1, // 0x0
          text2; // 0x4
  LPVOID unknownPointer3; // 0x8
  LPDWORD object4; // 0xc
  DWORD size5,            // 0x10
        unknownSize6;     // 0x14
};

/**
 *  Hooking thiscall using fastcall: http://tresp4sser.wordpress.com/2012/10/06/how-to-hook-thiscall-functions/
 *  - thiscall: this is in ecx and the first argument
 *  - fastcall: the first two parameters map to ecx and edx
 */
int __fastcall newHookFun(HookStruct *self, void *edx, DWORD arg1, DWORD arg2)
{
  Q_UNUSED(edx);
  if (!*self->object4) {
    qDebug() << QString::fromWCharArray((LPCWSTR)self);
    return 0;
  }
  //return oldHookFun(self, arg1, arg2);
  enum { role = Engine::ScenarioRole, signature = 1 }; // dummy signature
#ifdef DEBUG
  if (*self->object4)
    qDebug() << QString::fromWCharArray(self->text1) << ":"
             << (self->text1 == self->text2) << ":"
             << self->size5 << ";"
             << arg1 << ","
             << arg2 << ";"
             << " signature: " << QString::number(signature, 16);
#endif // DEBUG
  auto q = AbstractEngine::instance();

  //QString text = QString::fromWCharArray(self->text1, self->size5);
  QString text = "hell";
  text = q->dispatchTextW(text, signature, role);
  if (text.isEmpty())
    return 0;
    //return text.size() * 2;

  auto oldSize = self->size5;
  auto oldText = self->text1;
  //auto oldObject = self->object4;
  self->size5 = text.size();
  self->text1 = (LPCWSTR)text.utf16();
  //self->object4 = (LPDWORD)self->text1;

  int ret = oldHookFun(self, arg1, arg2); // supposed to equal size * 2

  self->size5 = oldSize;
  self->text1 = oldText;
  //self->object4 = oldObject;
  return ret;
}

} // unnamed namespace

/** Public class */

bool SiglusEngine::match() { return Engine::exists("SiglusEngine.exe"); }

bool SiglusEngine::attach()
{
  DWORD startAddress,
        stopAddress;
  if (!Engine::getCurrentMemoryRange(&startAddress, &stopAddress))
    return false;
  //enum { sub_esp = 0xec81 }; // caller pattern: sub esp = 0x81,0xec
  //DWORD addr = MemDbg::findCallerAddress((DWORD)::TextOutA, sub_esp, startAddress, stopAddress);
  DWORD addr = startAddress + 0xdb140; // 聖娼女 体験版
  //dmsg(*(BYTE *)addr); // supposed to be 0x55 = 85
  if (!addr)
    return false;
  return ::oldHookFun = detours::replace<hook_fun_t>(addr, ::newHookFun);
}

// EOF
