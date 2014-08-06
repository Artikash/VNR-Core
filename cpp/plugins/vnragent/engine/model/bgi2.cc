// bgi2.cc
// 8/5/2014 jichi
#include "engine/model/bgi2.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>

/**
 *  Sample game: 蒼の彼方のフォーリズム 体験版, reladdr = 0x32cd0
 *  Observations:
 *  - Text is in arg2
 *
 *  The main difference between this hook and BGI hook is that the text is in arg2 instead of arg3
 */
void BGI2Engine::hook(HookStack *stack)
{
  static QByteArray data_; // persistent storage, which makes this function not thread-safe

  LPCSTR text3 = (LPCSTR)stack->args[1]; // arg2

  DWORD split = stack->args[7]; // pseudo arg8
  auto sig = Engine::hashThreadSignature(stack->retaddr, split);

  data_ = EngineController::instance()->dispatchTextA(text3, sig, Engine::UnknownRole);
  stack->args[1] = (DWORD)data_.constData();
}

static ulong searchBGI2(ulong startAddress, ulong stopAddress)
{
  //return startAddress + 0x31850; // 世界と世界の真ん中 体験版
  const BYTE bytes[] = {
    0x3c, 0x20,       // 01312d8e   3c 20          cmp al,0x20     ; jichi: pattern starts
    0x7d, 0x58,       // 01312d90   7d 58          jge short 蒼の彼方.01312dea
    0x0f,0xbe,0xc0,   // 01312d92   0fbec0         movsx eax,al
    0x83,0xc0, 0xfe,  // 01312d95   83c0 fe        add eax,-0x2                             ;  switch (cases 2..8)
    0x83,0xf8, 0x06,  // 01312d98   83f8 06        cmp eax,0x6
    0x77, 0x4d        // 01312d9b   77 4d          ja short 蒼の彼方.01312dea
  };
  enum { hook_offset = 0x01312cd0 - 0x01312d8e }; // distance to the beginning of the function
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return 0;

  addr += hook_offset;
  enum : BYTE { push_ebp = 0x55 };  // 011d4c80  /$ 55             push ebp
  if (*(BYTE *)addr != push_ebp)
    return 0;

  return addr;
}

// BGI2 pattern also exists in BGI1
ulong BGI2Engine::search(ulong startAddress, ulong stopAddress)
{ return ::searchBGI2(startAddress, stopAddress); }

// EOF
