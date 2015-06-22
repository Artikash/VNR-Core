// retouch.cc
// 6/20/2015 jichi
#include "engine/model/retouch.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "memdbg/memsearch.h"
#include "disasm/disasm.h"
#include "winasm/winasmdef.h"
#include "winasm/winasmutil.h"
#include "winhook/hookcode.h"
#include "winhook/hookfun.h"
#include "winhook/hookcall.h"
#include <qt_windows.h>

#define DEBUG "retouch"
#include "sakurakit/skdebug.h"

namespace { // unnamed
namespace ScenarioHook {
namespace Private {
  bool hookBefore(winhook::hook_stack *s)
  {
    static QByteArray data_; // persistent storage, which makes this function not thread-safe
    // All threads including character names are linked together
    LPCSTR text = (LPCSTR)s->stack[1]; // arg1
    auto role =
      s->eax == 0 ?  Engine::NameRole : Engine::ScenarioRole;
      //s->ebx == 0 ? Engine::ScenarioRole :
      //Engine::OtherRole; // ruby is not skipped
    auto split = s->stack[0]; // retaddr
    auto sig = Engine::hashThreadSignature(role, split);
    data_ = EngineController::instance()->dispatchTextA(text, sig, role);
    s->stack[1] = (ulong)data_.constData(); // arg1
    return true;
  }
} // namespace Private

/**
 *  Sample games:
 *  - ららプリ
 *  - 箱庭ロジック
 *  All two function entries exist.
 *
 *  ?printSub@RetouchPrintManager@@AAE_NPBDAAVUxPrintData@@K@Z
 *  - Scenario
 *  0653F7DC   05F5717A  RETURN to resident.05F5717A from resident.?printSub@RetouchPrintManager@@AAE_NPBDAAVUxPrintData@@K@Z
 *  0653F7E0   082E9D88 ; jichi: text here
 *  0653F7E4   0653F914
 *  0653F7E8   00000000
 *  0653F7EC   62100CB4
 *  0653F7F0   01575A68
 *  0653F7F4   00000003
 *  0653F7F8   01574F48
 *  0653F7FC   00000000
 *  0653F800   000000E7
 *  0653F804   0653F8F0
 *  0653F808   00000081
 *  0653F80C   01670000
 *  0653F810   08599530
 *  0653F814   0653F8F0
 *  0653F818   00000000
 *  0653F81C   00000000
 *  0653F820   00009530
 *  0653F824   00000000
 *  0653F828   E70000E7
 *  0653F82C   01670000
 *  0653F830   08599940
 *  0653F834   085F3FF8
 *  0653F838   08613340
 *  0653F83C   00000338
 *  0653F840   01670000
 *  0653F844   00000100
 *  0653F848   0C599530
 *  0653F84C   0653F93C
 *
 *  EAX 082E2A08
 *  ECX 01575A68
 *  EDX 0653F914
 *  EBX 00000000
 *  ESP 0653F7DC
 *  EBP 00000000
 *  ESI 01575A68
 *  EDI 00000000
 *  EIP 05F56EB0 resident.?printSub@RetouchPrintManager@@AAE_NPBDAAVUxPrintData@@K@Z
 *
 *  EAX 07FFF358
 *  ECX 01535C80
 *  EDX 08156FD8
 *  EBX 00000000
 *  ESP 0656F824
 *  EBP 00000015
 *  ESI 01535C80
 *  EDI 080BC520
 *  EIP 05C20650 resident.?printSub@RetouchPrintManager@@AAE_NPBDAAVUxPrintData@@K@Z
 *
 *  Name
 *  EAX 00000000 ; jichi: used as split
 *  ECX 01575A68
 *  EDX 0653F944
 *  EBX 00000000
 *  ESP 0653F7DC
 *  EBP 00000000
 *  ESI 01575A68
 *  EDI 08480A20
 *  EIP 05F56EB0 resident.?printSub@RetouchPrintManager@@AAE_NPBDAAVUxPrintData@@K@Z
 *
 *  Ruby surface
 *  EAX 07FFF670
 *  ECX 01535C80
 *  EDX 0656F95C
 *  EBX 00000000
 *  ESP 0656F824
 *  EBP 00000015
 *  ESI 01535C80
 *  EDI 08134B10
 *  EIP 05C20650 resident.?printSub@RetouchPrintManager@@AAE_NPBDAAVUxPrintData@@K@Z
 *
 *  Ruby text
 *  EAX 07FFF778
 *  ECX 01535C80
 *  EDX 00000000
 *  EBX 00000092
 *  ESP 0656F824
 *  EBP 00000015
 *  ESI 01535C80
 *  EDI 08134B10
 *  EIP 05C20650 resident.?printSub@RetouchPrintManager@@AAE_NPBDAAVUxPrintData@@K@Z
 *
 */
bool attach() // attach scenario
{
  HMODULE hModule = ::GetModuleHandleA("resident.dll");
  if (!hModule)
    return false;
  // private: bool __thiscall RetouchPrintManager::printSub(char const *,class UxPrintData &,unsigned long)	0x10050650	0x00050650	2904 (0xb58)	resident.dll	C:\Local\箱庭ロジック\resident.dll	Exported Function
  const char *sig = "?printSub@RetouchPrintManager@@AAE_NPBDAAVUxPrintData@@K@Z";
  auto fun = ::GetProcAddress(hModule, sig);
  if (!fun)
    return false;
  return winhook::hook_before((ulong)fun, Private::hookBefore);
}
} // namespace ScenarioHook

namespace OtherHook {
namespace Private {
  bool hookBefore(winhook::hook_stack *s)
  {
    static QByteArray data_; // persistent storage, which makes this function not thread-safe
    // All threads including character names are linked together
    LPCSTR text = (LPCSTR)s->stack[1]; // arg1
    enum { role = Engine::OtherRole };
    auto split = s->stack[0]; // retaddr
    auto sig = Engine::hashThreadSignature(role, split);
    data_ = EngineController::instance()->dispatchTextA(text, sig, role);
    s->stack[1] = (ulong)data_.constData(); // arg1
    return true;
  }
} // namespace Private

bool attach() // attach scenario
{
  HMODULE hModule = ::GetModuleHandleA("resident.dll");
  if (!hModule)
    return false;
  // private: void __thiscall RetouchPrintManager::printSub(char const *,unsigned long,int &,int &)	0x10046560	0x00046560	2902 (0xb56)	resident.dll	C:\Local\箱庭ロジック\resident.dll	Exported Function
  const char *sig = "?printSub@RetouchPrintManager@@AAEXPBDKAAH1@Z";
  auto fun = ::GetProcAddress(hModule, sig);
  if (!fun)
    return false;
  return winhook::hook_before((ulong)fun, Private::hookBefore);
}
} // namespace OtherHook
} // unnamed namespace

/** Public class */

bool RetouchEngine::attach()
{
  if (!ScenarioHook::attach())
    return false;
  if (OtherHook::attach())
    DOUT("other text found");
  else
    DOUT("other text NOT FOUND");
  HijackManager::instance()->attachFunction((DWORD)::GetGlyphOutlineA);
  return true;
}

// EOF
