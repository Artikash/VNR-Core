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

using namespace std::placeholders; // for _1

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

ulong searchOtherAddress(ulong startAddress, ulong stopAddress)
{
  const char *pattern = "S_ASSIGN";
  BYTE bytes[] = {
    //0xc3,       // 005b6492   c3               retn
    //0x52,       // 005b6493   52               push edx
    0xe8, XX4,    // 005b6494   e8 77dc0000      call .005c4110     ; jichi: hook here
    0x84,0xc0,    // 005b6499   84c0             test al,al
    0x75, XX,     // 005b649b   75 16            jnz short .005b64b3
    0x68, XX4,    // 005b649d   68 d4757200      push .007275d4
    0xb9 //, XX4, // 005b64a2   b9 f0757200      mov ecx,.007275f0  ; ascii "S_ASSIGN"
    //0xe8, XX4   // 005b64a7   e8 84c8ffff      call .005b2d30
  };

  for (ulong addr = startAddress; addr < stopAddress;) {
    addr = MemDbg::matchBytes(bytes, sizeof(bytes), addr, stopAddress);
    if (!addr)
      return 0;
    addr += sizeof(bytes);
    DWORD ecx = *(DWORD *)addr;
    if (::strcmp((LPCSTR)ecx, pattern) == 0)
      return addr - sizeof(bytes);
  };
  return 0;
}

// - Hook -

struct TextHookBase
{
  struct TextArgument // first argument of the scenario hook
  {
    DWORD unknown1,
          unknown2;
    LPCSTR text;
    DWORD size; // text data size, length = size - 1
  };

  bool editable_; // for debugging only, whether text is not read-only
  QByteArray buffer_; // persistent storage, which makes this function not thread-safe
  TextArgument *arg_; // last argument
  LPCSTR text_; // last text
  DWORD size_; // last size

  TextHookBase()
    : editable_(true)
    , arg_(nullptr)
    , text_(nullptr)
    , size_(0)
  {}
};

class ScenarioHook : protected TextHookBase
{
public:
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

    // Stack structure
    // 0012F4BC   07EAFD48 ; text address
    // 0012F4C0   000002EC ; use this value as split
    // 0012F4C4   00000011
    // 0012F4C8   0012F510
    // 0012F4CC   00000012
    // 0012F4D0   00001BAA
    // 0012F4D4   00000012
    // 0012F4D8   06D2E24C
    // 0012F4DC   00581125  RETURN to .00581125 from .0057DC30
    //enum : WORD { ScenarioSplit = 0x84 };
    //DWORD split = s->stack[2];
    if (split != ScenarioSplit) // only translate the scenario thread
      return true;

    auto arg = (TextArgument *)s->stack[0]; // top of the stack
    LPCSTR text = arg->text;
    if (arg->size <= 1 || !text || !*text)
      return true;

    enum { role = Engine::ScenarioRole };
    DWORD sig = Engine::hashThreadSignature(role, split);
    //int size = arg->size; // size not used as not needed
    buffer_ = EngineController::instance()->dispatchTextA(text, sig, role);

    if (editable_) {
      arg_ = arg;
      text_ = arg->text;
      size_ = arg->size;
      arg->text = buffer_.constData(); // reset arg3
      arg->size = buffer_.size() + 1; // +1 for the nullptr
    }
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

class OtherHook : protected TextHookBase
{
public:
  bool hookBefore(winhook::hook_stack *s)
  {
    enum : WORD { OtherSplit = 0x46 }; // 0x440046 if use dword split
    DWORD splitBase = *(DWORD *)(s->edi + 0x284), // [edi + 0x284]
          split1 = *(WORD *)(splitBase - 0x4), // word [[edi + 0x284] - 0x4]
          split2 = *(WORD *)(splitBase - 0x8); // word [[edi + 0x284] - 0x8]
    if (split1 != OtherSplit || split2 <= 5) // split internal system messages
      return true;

    auto arg = (TextArgument *)s->stack[0]; // top of the stack
    LPCSTR text = arg->text;
    if (arg->size <= 1 || !text || !*text)
      return true;

    if (::strchr(text, '/') || ::strchr(text, '\\')) // skip text containing '/' or '\\' in it
      return true;

    enum { role = Engine::OtherRole };
    DWORD sig = Engine::hashThreadSignature(role, split2);
    buffer_ = EngineController::instance()->dispatchTextA(text, sig, role);

    if (editable_) {
      arg_ = arg;
      text_ = arg->text;
      size_ = arg->size;
      arg->text = buffer_.constData(); // reset arg3
      arg->size = buffer_.size() + 1; // +1 for the nullptr
    }
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
    char text[NameSize]; // 0x10
    DWORD type, // [[esp]+0x10]
          type2; // [[esp]+0x14]
  };

  auto arg = (NameArgument *)s->stack[0];
  if (arg->type2 != 0xf) // non 0xf is garbage text
    return true;

  char *text = arg->text;
  if (!text || !*text)
    return true;

  int role;
  long sig;
  if (arg->type == 0x6 || arg->type == 0xc) {
    role = Engine::NameRole;
    sig = Engine::NameThreadSignature;
  } else if (::strlen(text) <= 2)  // skip translating very short other text
    return true;
  else {
    role = Engine::OtherRole;
    sig = Engine::hashThreadSignature(role, arg->type);
  }

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

  {
    //ulong addr = 0x005c71e0;
    ulong addr = ::searchScenarioAddress(startAddress, stopAddress);
    if (!addr)
      return false;
    auto h = new ScenarioHook;
    if (!winhook::hook_both(addr,
        std::bind(&ScenarioHook::hookBefore, h, _1),
        std::bind(&ScenarioHook::hookAfter, h, _1))) {
      delete h;
      return false;
    }
    DOUT("text thread address" << QString::number(addr, 16));
  }

  if (ulong addr = ::searchOtherAddress(startAddress, stopAddress)) {
    auto h = new OtherHook;
    if (!winhook::hook_both(addr,
        std::bind(&OtherHook::hookBefore, h, _1),
        std::bind(&OtherHook::hookAfter, h, _1))) {
      DOUT("other text NOT FOUND");
      delete h;
    } else {
      DOUT("other text address" << QString::number(addr, 16));
    }
  }

  if (ulong addr = ::searchNameAddress(startAddress, stopAddress)) {
    if (winhook::hook_before(addr, ::nameHook))
      DOUT("name text address" << QString::number(addr, 16));
    else
      DOUT("name text NOT FOUND");
  }

  return true;
}

// EOF
