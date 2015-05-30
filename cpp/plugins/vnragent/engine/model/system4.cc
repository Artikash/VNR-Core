// system4.cc
// 5/25/2015 jichi
#include "engine/model/system4.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include <qt_windows.h>

#define DEBUG "system4"
#include "sakurakit/skdebug.h"

/**
 *  Sample game: Evenicle
 *  See: http://capita.tistory.com/m/post/256
 *
 */
//ulong System4Engine::search(ulong startAddress, ulong stopAddress)
//{
//  //const DWORD funcs[] = { // caller patterns
//  //  0xec81,     // sub esp = 0x81,0xec byte old majiro
//  //  0x83ec8b55  // mov ebp,esp, sub esp,*  new majiro
//  //};
//  //enum { FuncCount = sizeof(funcs) / sizeof(*funcs) };
//  //return MemDbg::findMultiCallerAddress((ulong)::TextOutA, funcs, FuncCount, startAddress, stopAddress);
//
//  // Pattern matching is not implemented
//  return 0x005C71E0;
//  //return 0x005C4110;    // FIXME: hook to this function will cause game to crash
//}

namespace { // unnamed

class ScenarioHook
{
  struct ScenarioArgument // first argument of the scenario hook
  {
    DWORD unknown1,
          unknown2;
    LPCSTR text;
    DWORD size; // text data size, length = size - 1
  };

  QByteArray buffer_; // persistent storage, which makes this function not thread-safe
  ScenarioArgument *arg_; // last argument
  LPCSTR text_; // last text
  DWORD size_; // last size

public:
  ScenarioHook()
    : arg_(nullptr)
    , text_(nullptr)
    , size_(0)
  {}

  bool hookBefore(winhook::hook_stack *s)
  {
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
    enum : WORD { ScenarioSplit = 0x27f2 };
    DWORD split = *(WORD *)(s->edi + 0xb0);
    if (split != ScenarioSplit) // only translate the scenario thread
      return true;

    auto arg = (ScenarioArgument *)s->stack[0]; // arg1
    LPCSTR text = arg->text;
    if (!text || !*text)
      return true;

    enum { role = Engine::ScenarioRole };
    enum { sig = Engine::ScenarioThreadSignature };

    //int size = arg->size; // size not used as not needed
    buffer_ = EngineController::instance()->dispatchTextA(text, sig, role);

    arg_ = arg;
    text_ = arg->text;
    size_ = arg->size;
    arg->text = buffer_.constData(); // reset arg3
    arg->size = buffer_.size() + 1; // +1 for the nullptr
    return true;
  }

  bool hookAfter(winhook::hook_stack *)
  {
    if (arg_) {
      arg_->text = text_;
      arg_->size = size_;
      arg_ = nullptr;
    }
    return true;
  }
};

} // unnamed namespace

bool System4Engine::attach()
{
  DWORD addr = 0x005C71E0;
  ScenarioHook *h = new ScenarioHook; // FIXME: this variable is never deleted
  return winhook::hook_both(addr
    , std::bind(&ScenarioHook::hookBefore, h, std::placeholders::_1)
    , std::bind(&ScenarioHook::hookAfter, h, std::placeholders::_1)
  );
}


// EOF
