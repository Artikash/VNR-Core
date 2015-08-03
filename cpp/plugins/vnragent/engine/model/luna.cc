// luna.cc
// 8/6/2015 jichi
#include "engine/model/luna.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
//#include "hijack/hijackmanager.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include <qt_windows.h>
#include <cstdint>

#define DEBUG "model/luna"
#include "sakurakit/skdebug.h"

namespace { // unnamed
namespace ScenarioHook {
namespace Private {
  bool hookBefore(winhook::hook_stack *s)
  {
    static QByteArray data_;
    auto text = (LPCSTR)s->stack[1]; // arg1
    if (!text || !*text)
      return true;
    enum { role = Engine::ScenarioRole };
    ulong retaddr = s->stack[0];
    auto sig = Engine::hashThreadSignature(role, retaddr);

    data_ = EngineController::instance()->dispatchTextA(text, role, sig);

    s->stack[1] = (ulong)data_.constData(); // arg1
    return true;
  }
} // namespace Private

/**
 *  Sample game: 悪堕ラビリンス
 *
 *  Debugging method: Hook to the first function that accessing the text
 *
 *  Text drawed here in arg1
 *
 *  00487C0E   CC               INT3
 *  00487C0F   CC               INT3
 *  00487C10   55               PUSH EBP
 *  00487C11   8BEC             MOV EBP,ESP
 *  00487C13   51               PUSH ECX
 *  00487C14   894D FC          MOV DWORD PTR SS:[EBP-0x4],ECX
 *  00487C17   8B45 FC          MOV EAX,DWORD PTR SS:[EBP-0x4]
 *  00487C1A   8B4D 08          MOV ECX,DWORD PTR SS:[EBP+0x8]
 *  00487C1D   8988 AC020000    MOV DWORD PTR DS:[EAX+0x2AC],ECX
 *  00487C23   8B55 FC          MOV EDX,DWORD PTR SS:[EBP-0x4]
 *  00487C26   D9EE             FLDZ
 *  00487C28   D99A B0020000    FSTP DWORD PTR DS:[EDX+0x2B0]
 *  00487C2E   8B45 FC          MOV EAX,DWORD PTR SS:[EBP-0x4]
 *  00487C31   8B88 84000000    MOV ECX,DWORD PTR DS:[EAX+0x84]
 *  00487C37   81E1 00000F00    AND ECX,0xF0000
 *  00487C3D   C1E9 10          SHR ECX,0x10
 *  00487C40   83F9 02          CMP ECX,0x2
 *  00487C43   75 21            JNZ SHORT .00487C66
 *  00487C45   8B55 FC          MOV EDX,DWORD PTR SS:[EBP-0x4]
 *  00487C48   8B82 AC020000    MOV EAX,DWORD PTR DS:[EDX+0x2AC]
 *  00487C4E   50               PUSH EAX
 *  00487C4F   8B4D FC          MOV ECX,DWORD PTR SS:[EBP-0x4]
 *  00487C52   8B89 88000000    MOV ECX,DWORD PTR DS:[ECX+0x88]
 *  00487C58   E8 0323FAFF      CALL .00429F60
 *  00487C5D   8B55 FC          MOV EDX,DWORD PTR SS:[EBP-0x4]
 *  00487C60   8982 B8020000    MOV DWORD PTR DS:[EDX+0x2B8],EAX
 *  00487C66   8BE5             MOV ESP,EBP
 *  00487C68   5D               POP EBP
 *  00487C69   C2 0400          RETN 0x4
 *  00487C6C   CC               INT3
 *  00487C6D   CC               INT3
 *  00487C6E   CC               INT3
 */
bool attach(ulong startAddress, ulong stopAddress) // attach scenario
{
  const uint8_t bytes[] = {
    0x89,0x88, 0xac,0x02,0x00,0x00, // 00487c1d   8988 ac020000    mov dword ptr ds:[eax+0x2ac],ecx
    0x8b,0x55, 0xfc,                // 00487c23   8b55 fc          mov edx,dword ptr ss:[ebp-0x4]
    0xd9,0xee                       // 00487c26   d9ee             fldz
  };
  ulong addr = MemDbg::matchBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;
  addr = MemDbg::findEnclosingAlignedFunction(addr);
  if (!addr)
    return false;
  return winhook::hook_before(addr, Private::hookBefore);
}
} // namespace ScenarioHook
} // unnamed namespace

/** Public class */

bool LunaSoftEngine::attach()
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;
  if (!ScenarioHook::attach(startAddress, stopAddress))
    return false;
  return true;
}

// EOF

