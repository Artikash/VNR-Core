// siglus.cc
// 5/25/2014 jichi
#include "engine/model/siglus.h"
#include "engine/enginedef.h"
#include "engine/engineenv.h"
#include "engine/enginehash.h"
#include "detoursutil/detoursutil.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>

//#define DEBUG "siglus"
#include "sakurakit/skdebug.h"

// Used to get function's return address
// http://stackoverflow.com/questions/8797943/finding-a-functions-address-in-c
//#pragma intrinsic(_ReturnAddress)

/** Private data */

namespace { // unnamed

/**
 *  Sample game: 聖娼女 体験版
 */
typedef int (HookClass::*hook_fun_t)(DWORD, DWORD);
hook_fun_t oldHookFun;

class HookClass {
  HookClass() {} // private constructor

  LPCWSTR text1, // 0x0
          text2; // 0x4
  LPVOID unknownPointer3, // 0x8
         unknownPointer4; // 0xc
  DWORD size5,            // 0x10
        unknownSize6;     // 0x14

public:
  int newHookFun(DWORD arg1, DWORD arg2)
  {
    return 0;
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
      return oldHookFun(arg1, arg2);
    else
      return 0; // TODO: investigate the return value
  }
};

} // unnamed namespace

/** Public class */

bool SiglusEngine::match() { return Engine::exists("SiglusEngine.exe"); }

bool SiglusEngine::attach()
{
  DWORD startAddress,
        stopAddress;
  if (!Engine::getCurrentMemoryRange(&startAddress, &stopAddress))
    return false;
  //enum { sub_esp = 0xec81 }; // caller pattern: sub esp = 0x81,0xec
  //DWORD addr = MemDbg::findCallerAddress((DWORD)::TextOutA, sub_esp, startAddress, stopAddress);
  DWORD addr = 0x013bac70; // 聖娼女 体験版
  if (!addr)
    return false;
  return ::oldHookFun = detours::replace<hook_fun_t>(addr, &HookClass::newHookFun);
}

// EOF
