// system4.cc
// 5/25/2015 jichi
#include "engine/model/system4.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>

#define DEBUG "system4"
#include "sakurakit/skdebug.h"

/**
 *  Sample game: Evenicle
 *  See: http://capita.tistory.com/m/post/256
 *
 */
ulong System4Engine::search(ulong startAddress, ulong stopAddress)
{
  //const DWORD funcs[] = { // caller patterns
  //  0xec81,     // sub esp = 0x81,0xec byte old majiro
  //  0x83ec8b55  // mov ebp,esp, sub esp,*  new majiro
  //};
  //enum { FuncCount = sizeof(funcs) / sizeof(*funcs) };
  //return MemDbg::findMultiCallerAddress((ulong)::TextOutA, funcs, FuncCount, startAddress, stopAddress);

  // Pattern matching is not implemented
  return 0x005C71E0;
  //return 0x005C4110;    // FIXME: hook to this function will cause game to crash
}

namespace { // unnamed

struct ScenarioArgument // first argument of the scenario hook
{
  DWORD unknown1,
        unknown2;
  LPCSTR text;
  DWORD size; // text data size, length = size - 1
};

} // unnamed namespace

/**
 *  Hook to replace scenario text.
 */
void System4Engine::hook(HookStack *stack)
{
  static QByteArray data_; // persistent storage, which makes this function not thread-safe

  // See ATcode patch:
  // 0070A12E   8B87 B0000000    MOV EAX,DWORD PTR DS:[EDI+0xB0]
  // 0070A134   66:8138 8400     CMP WORD PTR DS:[EAX],0x84
  // 0070A139   75 0E            JNZ SHORT .0070A149
  // 0070A13B   8378 EA 5B       CMP DWORD PTR DS:[EAX-0x16],0x5B
  // 0070A13F   75 08            JNZ SHORT .0070A149
  //DWORD testAddr = *(DWORD *)(stack->edi + 0xb0);
  //if (*(WORD *)testAddr != 0x84 || // compare [[edi+0xb0]] with 0x84
  //    *(DWORD *)(testAddr - 0x16) != 0x5b)   // compare [[edi+0xb0]- 0x16] with 0x5b ('[')
  //  return;
  enum : DWORD { ScenarioSplit = 0x27f2 };
  DWORD split = *(WORD *)(stack->edi + 0xb0);
  if (split != ScenarioSplit) // only translate the scenario thread
    return;

  auto arg = (ScenarioArgument *)stack->retaddr; // arg1
  LPCSTR text = arg->text;
  if (!text || !*text)
    return;

  enum { role = Engine::ScenarioRole };
  //DWORD sig = *(BYTE *)(stack->edi + 0x80);
  //auto sig = Engine::hashThreadSignature(stack->retaddr, split);
  enum { sig = Engine::ScenarioThreadSignature };

  //int size = arg->size; // size not used as not needed
  data_ = EngineController::instance()->dispatchTextA(text, sig, role);

  QByteArray *data = new QByteArray(data_);

  arg->text = data->constData(); // reset arg3
  arg->size = data->size() + 1; // +1 for the nullptr
}

// EOF
