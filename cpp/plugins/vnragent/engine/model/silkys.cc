// silkys.cc
// 5/17/2015 jichi
// About MES and Silky's arc
// See: http://www.dsl.gr.jp/~sage/mamitool/case/elf/aishimai.html
// MES: http://tieba.baidu.com/p/2068362185
#include "engine/model/silkys.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "winhook/hookcode.h"
#include "memdbg/memsearch.h"
#include "winasm/winasmdef.h"
#include <qt_windows.h>

#define DEBUG "elf"
#include "sakurakit/skdebug.h"

namespace { // unnamed
namespace ScenarioHook {
namespace Private {

  struct TextArgument
  {
    DWORD unknown; // should always be zero at runtime
    union {
      LPCSTR scenarioText;
      char nameText[1];
    };
  };

  QByteArray data_;
  TextArgument *scenarioArg_,
               *nameArg_;
  LPCSTR scenarioText_;

  enum { MaxNameSize = 100 };
  char nameText_[MaxNameSize + 1];

  bool hookBefore(winhook::hook_stack *s)
  {
    auto arg = (TextArgument *)s->stack[0]; // arg1
    auto arg2 = s->stack[1]; // arg2
    auto q = EngineController::instance();

    // Scenario
    if (arg2 == 0) {
      enum { role = Engine::ScenarioRole, sig = Engine::ScenarioThreadSignature };
      // Text from scenario could be bad when open backlog while the character is speaking
      auto text = arg->scenarioText;
      //if (!Engine::isAddressReadable(text))
      //  return true;
      data_ = q->dispatchTextA(text, sig, role);
      scenarioArg_ = arg;
      scenarioText_ = arg->scenarioText;
      arg->scenarioText = (LPCSTR)data_.constData();
    } else {
      enum { role = Engine::NameRole, sig = Engine::NameThreadSignature };
      auto text = arg->nameText;
      QByteArray oldData = text,
                 newData = q->dispatchTextA(oldData, sig, role);
      if (!newData.isEmpty()) {
        nameArg_ = arg;
        ::memcpy(nameText_, oldData.constData(), min(oldData.size() + 1, MaxNameSize));
        ::memcpy(text, newData.constData(), min(newData.size() + 1, MaxNameSize));
      }
    }
    return true;
  }

  bool hookAfter(winhook::hook_stack *)
  {
    if (scenarioArg_) {
      scenarioArg_->scenarioText = scenarioText_;
      scenarioArg_ = nullptr;
    }
    if (nameArg_) {
      ::strcpy(nameArg_->nameText, nameText_);
      nameArg_ = nullptr;
    }
    return true;
  }

} // namespace Private

/**
 *  Sample game: 堕ちていく新妻 trial
 *
 *  This function is found by backtracking GetGlyphOutlineA.
 *  There are two GetGlyphOutlineA, which are in the same function.
 *  That function are called by two other functions.
 *  The second function is hooked.
 *
 *  baseaddr = 08e0000
 *
 *  0096652E   CC               INT3
 *  0096652F   CC               INT3
 *  00966530   55               PUSH EBP
 *  00966531   8BEC             MOV EBP,ESP
 *  00966533   83EC 18          SUB ESP,0x18
 *  00966536   A1 00109F00      MOV EAX,DWORD PTR DS:[0x9F1000]
 *  0096653B   33C5             XOR EAX,EBP
 *  0096653D   8945 FC          MOV DWORD PTR SS:[EBP-0x4],EAX
 *  00966540   53               PUSH EBX
 *  00966541   8B5D 0C          MOV EBX,DWORD PTR SS:[EBP+0xC]
 *  00966544   56               PUSH ESI
 *  00966545   8B75 08          MOV ESI,DWORD PTR SS:[EBP+0x8]
 *  00966548   57               PUSH EDI
 *  00966549   6A 00            PUSH 0x0
 *  0096654B   894D EC          MOV DWORD PTR SS:[EBP-0x14],ECX
 *  0096654E   8B0D FCB7A200    MOV ECX,DWORD PTR DS:[0xA2B7FC]
 *  00966554   68 90D29D00      PUSH .009DD290                           ; ASCII "/Config/SceneSkip"
 *  00966559   895D F0          MOV DWORD PTR SS:[EBP-0x10],EBX
 *  0096655C   E8 2F4A0100      CALL .0097AF90
 *  00966561   83F8 01          CMP EAX,0x1
 *  00966564   0F84 E0010000    JE .0096674A
 *  0096656A   8B55 EC          MOV EDX,DWORD PTR SS:[EBP-0x14]
 *  0096656D   85DB             TEST EBX,EBX
 *  0096656F   75 09            JNZ SHORT .0096657A
 *  00966571   8B42 04          MOV EAX,DWORD PTR DS:[EDX+0x4]
 *  00966574   8B40 38          MOV EAX,DWORD PTR DS:[EAX+0x38]
 *  00966577   8945 F0          MOV DWORD PTR SS:[EBP-0x10],EAX
 *  0096657A   33C0             XOR EAX,EAX
 *  0096657C   C645 F8 00       MOV BYTE PTR SS:[EBP-0x8],0x0
 *  00966580   33C9             XOR ECX,ECX
 *  00966582   66:8945 F9       MOV WORD PTR SS:[EBP-0x7],AX
 *  00966586   3946 14          CMP DWORD PTR DS:[ESI+0x14],EAX
 *  00966589   0F86 BB010000    JBE .0096674A
 *
 *  Scenario stack:
 *
 *  002FF9DC   00955659  RETURN to .00955659 from .00966530
 *  002FF9E0   002FFA10  ; jichi: text in [arg1+4]
 *  002FF9E4   00000000  ; arg2 is zero
 *  002FF9E8   00000001
 *  002FF9EC   784B8FC7
 *
 *  Name stack:
 *
 *  002FF59C   00930A76  RETURN to .00930A76 from .00966530
 *  002FF5A0   002FF5D0 ; jichi: text in [arg1+4]
 *  002FF5A4   004DDEC0 ; arg2 is a pointer
 *  002FF5A8   00000001
 *  002FF5AC   784B8387
 *  002FF5B0   00000182
 *  002FF5B4   00000000
 *
 *  Scenario and Name are called by different callers.
 */
bool attach()
{
  ulong startAddress, stopAddress;
  if (!Engine::getCurrentMemoryRange(&startAddress, &stopAddress))
    return false;

  const char *msg = "/Config/SceneSkip";
  ulong addr = MemDbg::findBytes(msg, ::strlen(msg) + 1, startAddress, stopAddress); // +1 to include \0 at the end
  if (!addr)
    return false;
  addr = MemDbg::findPushAddress(addr, startAddress, stopAddress);
  if (!addr)
    return false;
  addr = MemDbg::findEnclosingAlignedFunction(addr);
  if (!addr)
    return false;

  //ulong addr = MemDbg::findCallerAddressAfterInt3(startAddress, stopAddress);
  //if (!addr)
  //  return false;
  //addr = MemDbg::findLastNearCallAddress(addr);
  //if (!addr)
  //  return false;

  return winhook::hook_before(addr, Private::hookBefore);

  int count = 0;
  auto fun = [&count](ulong addr) -> bool {
    count += winhook::hook_both(addr, Private::hookBefore, Private::hookAfter);
    return true; // replace all functions
  };
  MemDbg::iterNearCallAddress(fun, addr, startAddress, stopAddress);
  DOUT("call number =" << count);
  return count;
}

} // namespace ScenarioHook
} // unnamed namespace

bool SilkysEngine::attach()
{
  if (!ScenarioHook::attach())
    return false;
  HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineA);
  return true;
}

// EOF
