// rejet.cc
// 5/25/2014 jichi
#include "engine/model/rejet.h"
#include "engine/enginedef.h"
#include "engine/engineenv.h"
#include "engine/enginehash.h"
#include "detoursutil/detoursutil.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>
#include <QtCore/QStringList>

//#define DEBUG "rejet"
#include "sakurakit/skdebug.h"

// Used to get function's return address
// http://stackoverflow.com/questions/8797943/finding-a-functions-address-in-c
//#pragma intrinsic(_ReturnAddress)

/** Private data */

namespace { // unnamed

/**
 *  Sample game: 剣が君
 *  .text:0044D620 ; int __stdcall sub_44D620(int, void *, unsigned __int8 *, void *, int)
 *  .text:0044D620 sub_44D620      proc near               ; CODE XREF: sub_438C30+96p
 *  .text:0044D620                                         ; sub_45A190+8Cp
 *  Observations from 剣が君:
 *  - arg1: Scenario text
 *  - arg2: role name
 *  - arg3: unknown string
 *  - arg4: role name
 *  - arg5: size or width, not sure
 *  - return: unknown
 *
 * 428cc6 - 428c30
 * 44a21c - 44a190 - 44a178 - 449ae2 - 449aa0
 */
typedef int (__stdcall *hook_fun_t)(LPCSTR, LPCSTR, LPCSTR, LPCSTR, int);
hook_fun_t oldHookFun;

// CHECKPOINT 5/25/2014: This is not the correct function to inject
int __stdcall newHookFun(LPCSTR text1, LPCSTR text2, LPCSTR text3, LPCSTR text4, int size5)
{
  return 0;
  //return oldHookFun(text1, text2, text3, text4, size5);
  // Compute ITH signature
  //DWORD returnAddress = (DWORD)_ReturnAddress();
  //      //split = splitOf((DWORD *)fontName1); split is not used
  //auto signature = Engine::hashThreadSignature(returnAddress);
  enum { role = Engine::ScenarioRole, signature = 1 }; // dummy signature
#ifdef DEBUG
  qDebug() << QString::fromLocal8Bit(text1) << ":"
           << QString::fromLocal8Bit(text2) << ":"
           << QString::fromLocal8Bit(text3) << ":"
           << QString::fromLocal8Bit(text4) << ":"
           << size5 << ";"
           << " signature: " << QString::number(signature, 16);
#endif // DEBUG
  //return oldHookFun(text1, text2, text3, text4, size5);
  auto q = AbstractEngine::instance();
  QByteArray data = q->dispatchTextA(text1, signature, role);
  if (!data.isEmpty())
    return oldHookFun(data, text2, text3, text4, size5);
  else
    return 0; // TODO: investigate the return value
}

} // unnamed namespace

/** Public class */

bool RejetEngine::match()
{ return Engine::exists(QStringList() << "gd.dat" << "pf.dat" << "sd.dat"); }

bool RejetEngine::attach()
{
  DWORD startAddress,
        stopAddress;
  if (!Engine::getCurrentMemoryRange(&startAddress, &stopAddress))
    return false;
  //enum { sub_esp = 0xec81 }; // caller pattern: sub esp = 0x81,0xec
  //DWORD addr = MemDbg::findCallerAddress((DWORD)::TextOutA, sub_esp, startAddress, stopAddress);
  DWORD addr = startAddress + 0x4d620; // 剣が君
  //DWORD addr = startAddress + 0x38c30
  if (!addr)
    return false;
  return ::oldHookFun = detours::replace<hook_fun_t>(addr, ::newHookFun);
}

// EOF
