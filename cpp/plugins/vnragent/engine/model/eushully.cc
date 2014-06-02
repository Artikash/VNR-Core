// eushully.cc
// 6/1/2014 jichi
// See: http://bbs.sumisora.org/read.php?tid=11044256
#include "engine/model/eushully.h"
#include "engine/enginedef.h"
#include "engine/engineenv.h"
#include "engine/enginehash.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>
#include <string>

#include "debug.h"
#define DEBUG "Eushully"
#include "sakurakit/skdebug.h"

/** Private data */

namespace { // unnamed

/**
 *  Sample game: 姫狩りダンジョンマイスター体験版
 *  Caller of the last GetTextExtentPoint32A: 0x451170
 *  Observations:
 *  - arg1: 0
 *  - arg2: LPCSTR
 *  - arg3: 0
 *  - arg4: 0
 *  - arg5: LPVOID  to unknown
 *  - return: TRUE(1) if succeed
 */
//typedef int (__cdecl *hook_fun_t)(int, LPCSTR); //, int, int, LPVOID);
typedef DWORD hook_fun_t;
hook_fun_t oldHookFun;

__declspec(noinline)
LPCSTR dispatchText(LPCSTR text)
{
  enum { role = Engine::ScenarioRole, signature = Engine::SingleTextSignature };
  static QByteArray ret; // persistent storage, which makes this function not thread-safe
  ret = AbstractEngine::instance()->dispatchTextA(text, signature, role);
  return (LPCSTR)ret.constData();
}

LPCSTR test(LPCSTR text)
{
  // CHECKPOINT: Cannot access heap?!
  new char[1000];
  //malloc(1000);
  return text;
}

//int newHookFun(int arg1, LPCSTR text2) //, int arg3, int arg4, LPVOID arg5)
__declspec(naked) // jichi 10/2/2013: No prolog and epilog
int newHookFun()
{
  //static DWORD lastArg2;
  __asm
  {
    push [esp+8]    ; arg2
    call test
    add esp,4       ; pop arg2
    mov [esp+8],eax ; modify arg2
    jmp oldHookFun
  }
}

} // unnamed namespace

/** Public class */

bool EushullyEngine::match() { return Engine::exists("AGERC.DLL"); }

bool EushullyEngine::attach()
{
  DWORD startAddress,
        stopAddress;
  if (!Engine::getCurrentMemoryRange(&startAddress, &stopAddress))
    return false;
  enum { sub_esp = 0xec81 }; // caller pattern: sub esp = 0x81,0xec
  DWORD addr; // = MemDbg::findCallerAddress((DWORD)::TextOutA, sub_esp, startAddress, stopAddress);
  // Note: ITH will mess up this value
  //addr = 0x41af90; // レミニセンス function address
  addr = 0x451170; // 姫狩りダンジョンマイスター体験版
  dmsg(*(WORD *)addr); // supposed to be 0xff6a
  if (!addr)
    return false;
  return ::oldHookFun = replaceFunction<hook_fun_t>(addr, ::newHookFun);
}

// EOF
