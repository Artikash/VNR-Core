// malie.cc
// 8/8/2015 jichi
#include "engine/model/malie.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
//#include "hijack/hijackmanager.h"
#include "util/textutil.h"
#include "winhook/hookcode.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>
#include <cstdint>

#define DEBUG "model/malie"
#include "sakurakit/skdebug.h"

namespace { // unnamed
namespace ScenarioHook {
namespace Private {
  bool hookBefore(winhook::hook_stack *s)
  {
    static QString text_;
    auto text = (LPCWSTR)s->stack[1];
    if (!text || !*text || Util::allAscii(text))
      return true;

    auto role = Engine::ScenarioRole;
    auto retaddr = s->stack[0];
    auto sig = Engine::hashThreadSignature(role, retaddr);
    sig = retaddr;

    QString oldText = QString::fromWCharArray(text),
            newText = EngineController::instance()->dispatchTextW(oldText, role, sig);
    if (newText.isEmpty() || newText == oldText)
      return true;
    text_ = newText;
    s->stack[1] = (ulong)text_.utf16();
    return true;
  }

} // namespace Private

/**
 *  Sample game: シルヴァリオ ヴェンデッタ
 *
 *  Text in arg1.
 *  Type text in arg2?
 *
 *  00651F5F   90                   NOP
 *  00651F60   83EC 70              SUB ESP,0x70
 *  00651F63   68 F8547F00          PUSH .007F54F8
 *  00651F68   FF15 F8206900        CALL DWORD PTR DS:[0x6920F8]             ; ntdll.RtlEnterCriticalSection
 *  00651F6E   8B4424 74            MOV EAX,DWORD PTR SS:[ESP+0x74]
 *  00651F72   8D4C24 00            LEA ECX,DWORD PTR SS:[ESP]
 *  00651F76   51                   PUSH ECX
 *  00651F77   894424 04            MOV DWORD PTR SS:[ESP+0x4],EAX
 *  00651F7B   C74424 08 00000000   MOV DWORD PTR SS:[ESP+0x8],0x0
 *  00651F83   C74424 6C 00000000   MOV DWORD PTR SS:[ESP+0x6C],0x0
 *  00651F8B   C74424 50 01000000   MOV DWORD PTR SS:[ESP+0x50],0x1
 *  00651F93   C74424 54 FFFFFFFF   MOV DWORD PTR SS:[ESP+0x54],-0x1
 *  00651F9B   E8 10FAFFFF          CALL .006519B0
 *  00651FA0   83C4 04              ADD ESP,0x4
 *  00651FA3   C74424 4C 00000000   MOV DWORD PTR SS:[ESP+0x4C],0x0
 *  00651FAB   68 F8547F00          PUSH .007F54F8
 *  00651FB0   FF15 44226900        CALL DWORD PTR DS:[0x692244]             ; ntdll.RtlLeaveCriticalSection
 *  00651FB6   8B4424 68            MOV EAX,DWORD PTR SS:[ESP+0x68]
 *  00651FBA   83C4 70              ADD ESP,0x70
 *  00651FBD   C3                   RETN
 *  00651FBE   90                   NOP
 *  00651FBF   90                   NOP
 */
bool attach(ulong startAddress, ulong stopAddress)
{
  const uint8_t bytes[] = {
    0x89,0x44,0x24, 0x04,                       // 00651f77   894424 04            mov dword ptr ss:[esp+0x4],eax
    0xc7,0x44,0x24, 0x08, 0x00,0x00,0x00,0x00,  // 00651f7b   c74424 08 00000000   mov dword ptr ss:[esp+0x8],0x0
    0xc7,0x44,0x24, 0x6c, 0x00,0x00,0x00,0x00,  // 00651f83   c74424 6c 00000000   mov dword ptr ss:[esp+0x6c],0x0
    0xc7,0x44,0x24, 0x50, 0x01,0x00,0x00,0x00,  // 00651f8b   c74424 50 01000000   mov dword ptr ss:[esp+0x50],0x1
    0xc7,0x44,0x24, 0x54, 0xff,0xff,0xff,0xff   // 00651f93   c74424 54 ffffffff   mov dword ptr ss:[esp+0x54],-0x1
  };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;
  addr = MemDbg::findEnclosingAlignedFunction(addr);
  if (!addr)
    return false;
  return winhook::hook_before(addr, Private::hookBefore);
}
} // namespace ScenarioHook
} // unnamed namespace

bool MalieEngine::attach()
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;
  if (!ScenarioHook::attach(startAddress, stopAddress))
    return false;
  //HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineW);
  return true;
}

// EOF
