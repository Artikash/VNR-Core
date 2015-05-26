// system4.cc
// 5/25/2015 jichi
#include "engine/model/system4.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>

//#define DEBUG "system4"
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
  return 0x005C4110;    // FIXME: hook to this function will cause game to crash
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

  auto arg = (ScenarioArgument *)stack->args[0]; // arg1
  LPCSTR text = arg->text;
  if (!text || !*text)
    return;

  enum { role = Engine::ScenarioRole };
  DWORD split = 0;
  auto sig = Engine::hashThreadSignature(stack->retaddr, split);

  //int size = arg->size; // size not used as not needed
  data_ = EngineController::instance()->dispatchTextA(text, sig, role);

  //dmsg(QString::fromLocal8Bit(ret));
  arg->text = data_.constData(); // reset arg3
  arg->size = data_.size() + 1; // +1 for the nullptr
}

// EOF
