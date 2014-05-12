// bgi.cc
// 5/11/2014 jichi
#include "engine/model/bgi.h"
#include "engine/enginedef.h"
#include "engine/engineenv.h"
#include "detoursutil/detoursutil.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>

#define DEBUG "test"

#include "sakurakit/skdebug.h"
#ifdef DEBUG
# include "debug.h"
#endif // DEBUG

/** Private class */

// Make this as class instead of namespace so that it can be specified as friend
class BGIEnginePrivate
{
  typedef BGIEngine Q;
public:
  /**
   *  世界と世界の真ん中 体験版, relative address: 0x31850;
   *
   *  TODO: Figure the meaning of the parameters and the return value
   *  ? __cdecl sub_31850(DWORD arg1, DWORD arg2, LPCSTR arg3, DWORD arg4)
   *  - arg1: address
   *  - arg2: address
   *  - arg3: string
   *  - arg4: uknown
   */
  static DWORD hookAddress;
  typedef int (__cdecl *hook_fun_t)(DWORD, DWORD, LPCSTR, DWORD); // __stdcall will crash the game
  static hook_fun_t oldHook;

  static int __cdecl newHook(DWORD arg1, DWORD arg2, LPCSTR arg3, DWORD arg4)
  {
    auto q = static_cast<Q *>(AbstractEngine::instance());
    auto role = Engine::TextRole; // TODO
    const char *repl = q->exchangeTextA(arg3, role);
    // TODO: Investigate the return value
    return repl ? oldHook(arg1, arg2, repl, arg4) : 0;
  }
};
DWORD BGIEnginePrivate::hookAddress;
BGIEnginePrivate::hook_fun_t BGIEnginePrivate::oldHook;

/** Public class */

bool BGIEngine::match() { return Engine::glob("BGI.*"); }

bool BGIEngine::attach()
{
  //DWORD dwTextOutA = Engine::getModuleFunction("gdi32.dll", "TextOutA");
  //if (!dwTextOutA)
  //  return false;
  //D::oldHook = dwTextOutA;
  //detours::replace((detours::address_t)dwTextOutA, (detours::address_t)MyTextOutA);
  DWORD startAddress,
        stopAddress;
  if (!Engine::getCurrentMemoryRange(&startAddress, &stopAddress))
    return false;
  //D::hookAddress = MemDbg::findCallerAddress(dwTextOutA, 0xec81, startAddress, stopAddress);
  DWORD reladdr = 0x31850; // 世界と世界の真ん中 体験版
  D::hookAddress = startAddress + reladdr;
  if (!D::hookAddress)
    return false;
  D::oldHook = detours::replace<D::hook_fun_t>(D::hookAddress, D::newHook);
  return D::oldHook;
}

bool BGIEngine::detach()
{
  if (!D::hookAddress || !D::oldHook)
    return false;
  bool ok = detours::restore<D::hook_fun_t>(D::hookAddress, D::oldHook);
  D::hookAddress = 0;
  return ok;
}

// EOF
