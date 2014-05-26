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
  LPWORD *flag;     // 0xc
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
  auto q = AbstractEngine::instance();

  if (self->flag && !*self->flag) {
#ifdef DEBUG
    qDebug() << QString::fromWCharArray(self->text);
#endif // DEBUG
    QString text = QString::fromWCharArray(self->text, self->size);
    text = q->dispatchTextW(text, signature, role);
    if (text.isEmpty())
      return 0;

    // FIXME: replacement is not implemented
    return oldHookFun(self, arg1, arg2);
  }

  //return oldHookFun(self, arg1, arg2);
  enum { role = Engine::ScenarioRole, signature = 1 }; // dummy signature
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

  QString text = QString::fromWCharArray(self->text[0], self->size);
  text = q->dispatchTextW(text, signature, role);
  if (text.isEmpty())
    return 0;
    //return text.size() * 2;

  auto oldSize = self->size;
  auto oldText = self->text[0];
  self->size = text.size();
  self->text[0] = (LPCWSTR)text.utf16();

  int ret = oldHookFun(self, arg1, arg2); // supposed to equal size * 2

  self->size = oldSize;
  self->text[0] = oldText;
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
