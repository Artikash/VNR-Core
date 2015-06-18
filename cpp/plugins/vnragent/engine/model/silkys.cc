// silkys.cc
// 5/17/2015 jichi
// About MES and Silky's arc
// See: http://www.dsl.gr.jp/~sage/mamitool/case/elf/aishimai.html
// MES: http://tieba.baidu.com/p/2068362185
#include "engine/model/silkys.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include "winasm/winasmdef.h"
#include <qt_windows.h>

#define DEBUG "elf"
#include "sakurakit/skdebug.h"

namespace { // unnamed

namespace ScenarioHook {
namespace Private {

  enum { ShortTextCapacity = 0x10 }; // 16

  struct TextArgument
  {
    DWORD unknown; // should always be zero at runtime
    union {
      LPCSTR longText;
      char shortText[ShortTextCapacity];
    };
    int size; // text size
  };

  QByteArray data_;
  TextArgument *arg_,
               argValue_;

  bool hookBefore(winhook::hook_stack *s)
  {
    auto arg = (TextArgument *)s->stack[0]; // arg1
    if (arg->size <= 0 || arg->size > Engine::MaxTextSize)
      return true;

    auto arg2 = s->stack[1]; // arg2
    auto role = arg2 == 0 ? Engine::ScenarioRole : Engine::NameRole;
    auto sig = Engine::hashThreadSignature(role);

    auto q = EngineController::instance();

    arg_ = arg;
    argValue_ = *arg;
    if (arg->size >= ShortTextCapacity && Engine::isAddressReadable(arg->longText, arg->size)) {
      data_ = q->dispatchTextA(arg->longText, sig, role);
      arg->longText = (LPCSTR)data_.constData();
      arg->size = data_.size(); // not needed and could crash on the other hand
    } else {
      auto text = arg->shortText;
      QByteArray data(text, arg->size);
      data = q->dispatchTextA(data, sig, role);
      arg->size = max(data.size(), ShortTextCapacity - 1); // truncate
      ::memcpy(text, data.constData(), min(data.size() + 1, ShortTextCapacity));
    }
    return true;
  }

  bool hookAfter(winhook::hook_stack *)
  {
    if (arg_)  {
      *arg_ = argValue_;
      arg_ = nullptr;
    }
    return true;
  }

} // namespace Private

/** jichi: 6/17/2015
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

  const BYTE bytes[] = {
    0x66,0x89,0x45, 0xf9,   // 00a1a062   66:8945 f9       mov word ptr ss:[ebp-0x7],ax
    0x39,0x47, 0x14         // 00a1a066   3947 14          cmp dword ptr ds:[edi+0x14],eax
  };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;

  //const char *msg = "\0/Config/SceneSkip";
  //ulong addr = MemDbg::findBytes(msg, ::strlen(msg + 1) + 2, startAddress, stopAddress); // +1 to include \0 at the end
  //if (!addr)
  //  return false;
  //addr += 1; // skip leading "\0"
  //addr = MemDbg::findPushAddress(addr, startAddress, stopAddress);
  //if (!addr)
  //  return false;

  addr = MemDbg::findEnclosingAlignedFunction(addr);
  if (!addr)
    return false;

  //ulong addr = MemDbg::findCallerAddressAfterInt3((ulong)::GetGlyphOutlineA, startAddress, stopAddress);
  //if (!addr)
  //  return false;
  //addr = MemDbg::findLastNearCallAddress(addr, startAddress, stopAddress);
  //if (!addr)
  //  return false;

  //DOUT(addr);

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
