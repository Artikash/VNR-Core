// test.cc
// 5/11/2014 jichi
#include "engine/model/test.h"
#include "engine/enginedef.h"
#include "engine/engineenv.h"
#include "detoursutil/detoursutil.h"
#include "memdbg/memsearch.h"
#include "winmutex/winmutex.h"
#include <qt_windows.h>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>

#define DEBUG "test"

#include "sakurakit/skdebug.h"
#ifdef DEBUG
# include "debug.h"
#endif // DEBUG

/** Private class */

// Make this as class instead of namespace so that it can be specified as friend
class TestEnginePrivate
{
  typedef TestEngine Q;

  typedef win_mutex<CRITICAL_SECTION> mutex_type;
  static mutex_type mutex;

public:

  static DWORD hookAddress;
  typedef int (__cdecl *hook_fun_t)(DWORD, DWORD, LPCSTR, DWORD); // __stdcall will crash the game
  static hook_fun_t oldHook;

  // 世界と世界の真ん中 体験版
  static int __cdecl newHook(DWORD arg1, DWORD arg2, LPCSTR arg3, DWORD arg4) //, DWORD arg4)
  {
    QByteArray data; // crash here
    arg3 = "hello";
    return oldHook(arg1, arg2, arg3, arg4);
  }
};
TestEnginePrivate::mutex_type TestEnginePrivate::mutex;
DWORD TestEnginePrivate::hookAddress;
TestEnginePrivate::hook_fun_t TestEnginePrivate::oldHook;

/** Public class */

bool TestEngine::match() { return true; }

bool TestEngine::inject()
{
  //DWORD dwTextOutA = Engine::getModuleFunction("gdi32.dll", "TextOutA");
  //if (!dwTextOutA)
  //  return false;
  //D::oldHook = dwTextOutA;
  //detours::replace((detours::address_t)dwTextOutA, (detours::address_t)MyTextOutA);
  DWORD startAddress, stopAddress;
  if (!Engine::getMemoryRange(nullptr, &startAddress, &stopAddress))
    return false;
  //D::hookAddress = MemDbg::findCallerAddress(dwTextOutA, 0xec81, startAddress, stopAddress);
  DWORD reladdr = 0x31850; // 世界と世界の真ん中 体験版
  D::hookAddress = startAddress + reladdr;
  //D::hookAddress = 0x405474;
  if (!D::hookAddress)
    return false;
  D::oldHook = detours::replace<D::hook_fun_t>(D::hookAddress, D::newHook);
  return true;
}

bool TestEngine::unload()
{
  if (!D::hookAddress || !D::oldHook)
    return false;
  bool ok = detours::restore<D::hook_fun_t>(D::hookAddress, D::oldHook);
  D::hookAddress = 0;
  return ok;
}

// EOF
