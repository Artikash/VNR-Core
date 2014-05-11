// kirikiri.cc
// 5/10/2014 jichi
// See: http://ja.wikipedia.org/wiki/吉里吉里2
#include "engine/model/kirikiri.h"
#include "engine/enginedef.h"
#include "engine/engineenv.h"
//#include "detoursutil/detoursutil.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>

//#define DEBUG "kirkiri
#include "sakurakit/skdebug.h"

/** Privat class */

class KiriKiriEnginePrivate
{
};

/** Public class */

bool KiriKiriEngine::match()
{
  // TODO: Add check for SearchResourceString
  //if (IthFindFile(L"*.xp3") || Util::SearchResourceString(L"TVP(KIRIKIRI)")) {
  //  InsertKiriKiriHook();
  //  return yes;
  //}
  return Engine::glob("*.xp3");
}

bool KiriKiriEngine::inject()
{
  DWORD dwTextOutA = Engine::getModuleFunction("gdi32.dll", "TextOutA");
  if (!dwTextOutA)
    return false;
  DWORD startAddress, stopAddress;
  if (!Engine::getMemoryRange(nullptr, &startAddress, &stopAddress))
    return false;
  //D::hookAddress = MemDbg::findCallerAddress(dwTextOutA, 0xec81, startAddress, stopAddress);
  // 淫辱選挙戦 ～白濁に染まる会長選～
  // Note: ITH will mess up this value
  // KiriKiri1 (GetGlyphOutlineW): 0x541b24, dword arg0, dword arg1
  // KiriKiri2 (GetTextExtentPoint32W): 0x549a47, this is not a function!
  //D::hookAddress = 0x41af90;
  //if (!D::hookAddress)
  //  return false;
  //D::oldHook = detours::replace<D::hook_fun_t>(D::hookAddress, D::newHook);
  //addr = 0x41f650; // 2
  //addr = 0x416ab0;
  //D::oldtest = detours::replace<D::test_fun_t>(addr, D::newtest);
  return true;
}

bool KiriKiriEngine::unload()
{
  //if (!D::hookAddress || !D::oldHook)
  //  return false;
  //bool ok = detours::restore<D::hook_fun_t>(D::hookAddress, D::oldHook);
  //D::hookAddress = 0;
  //return ok;
  return false;
}

// EOF
