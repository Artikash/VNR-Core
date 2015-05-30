// system4.cc
// 5/25/2015 jichi
#include "engine/model/system4.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "ntinspect/ntinspect.h"
#include "winhook/hookcode.h"
#include <qt_windows.h>

#define DEBUG "system4"
#include "sakurakit/skdebug.h"

/**
 *  Sample game: Evenicle
 *  See: http://capita.tistory.com/m/post/256
 *
 */
namespace { // unnamed

// - Search -

bool getMemoryRange(ulong *startAddress, ulong *stopAddress)
{
  //bool patched = IthCheckFile(L"AliceRunPatch.dll");
  bool patched = ::GetModuleHandleA("AliceRunPatch.dll");
  return patched ?
      NtInspect::getModuleMemoryRange(L"AliceRunPatch.dll", startAddress, stopAddress) :
      NtInspect::getCurrentMemoryRange(startAddress, stopAddress);
}

ulong searchScenarioAddress(ulong startAddress, ulong stopAddress)
{
  const BYTE bytes[] = {
    0xe8, XX4,              // 005c71e0   e8 2bcfffff      call .005c4110  ; original function call
    0xeb, 0xa5,             // 005c71e5  ^eb a5            jmp short .005c718c
    0x8b,0x47, 0x08,        // 005c71e7   8b47 08          mov eax,dword ptr ds:[edi+0x8]
    0x8b,0x4f, 0x0c         // 005c71ea   8b4f 0c          mov ecx,dword ptr ds:[edi+0xc]
  };
  return MemDbg::matchBytes(bytes, sizeof(bytes), startAddress, stopAddress);
}

ulong searchNameAddress(ulong startAddress, ulong stopAddress)
{
  const BYTE bytes[] = {
    0xe8, XX4,              // 004eeb34   e8 67cb0100      call .0050b6a0  ; jichi: hook here
    0x39,0x6c,0x24, 0x28,   // 004eeb39   396c24 28        cmp dword ptr ss:[esp+0x28],ebp
    0x72, 0x0d,             // 004eeb3d   72 0d            jb short .004eeb4c
    0x8b,0x4c,0x24, 0x14,   // 004eeb3f   8b4c24 14        mov ecx,dword ptr ss:[esp+0x14]
    0x51,                   // 004eeb43   51               push ecx
    0xe8 //, XX4,           // 004eeb44   e8 42dc1900      call .0068c78b
  };
  return MemDbg::matchBytes(bytes, sizeof(bytes), startAddress, stopAddress);
}

// - Hook -

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

    auto arg = (ScenarioArgument *)s->stack[0]; // top of the stack
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

bool nameHook(winhook::hook_stack *s)
{
  enum { NameSize = 0x10 };
  struct NameArgument // first argument of the name hook
  {
    char text[NameSize];
    DWORD split; // use [[esp]+0x10] as split
  };

  auto arg = (NameArgument *)s->stack[0];

  enum : DWORD { NameSplit = 0x6, ScenarioSplit = 0x2 };
  if (arg->split != NameSplit) // only translate the name thread
    return true;

  char *text = arg->text;
  if (!text || !*text)
    return true;

  enum { role = Engine::NameRole };
  enum { sig = Engine::NameThreadSignature };

  QByteArray buffer_ = EngineController::instance()->dispatchTextA(text, sig, role);

  ::strncpy(text, buffer_.constData(), NameSize - 1);
  text[NameSize - 1] = 0;
  return true;
}

} // unnamed namespace

bool System4Engine::attach()
{
  ulong startAddress, stopAddress;
  if (!::getMemoryRange(&startAddress, &stopAddress))
    return false;

  //ulong addr = 0x005c71e0;
  ulong addr = ::searchScenarioAddress(startAddress, stopAddress);
  if (!addr)
    return false;
  ScenarioHook *h = new ScenarioHook;
  if (!winhook::hook_both(addr,
        std::bind(&ScenarioHook::hookBefore, h, std::placeholders::_1),
        std::bind(&ScenarioHook::hookAfter, h, std::placeholders::_1))) {
    delete h;
    return false;
  }

  if (ulong addr = ::searchNameAddress(startAddress, stopAddress))
    winhook::hook_before(addr, ::nameHook);
  return true;
}

// EOF
