// bgi.cc
// 5/11/2014 jichi
#include "engine/model/bgi.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include <qt_windows.h>

// BGI2 pattern also exists in BGI1
bool BGIEngine::attach() { return attachBGIType2() || attachBGIType1(); }

/**
 *  Type 1: Sample game: FORTUNE ARTERIAL, 0x4207E0
 *
 *  ? __cdecl sub_4207E0 proc near
 *  - arg1: address
 *  - arg2: address
 *  - arg3: string, LPCSTR
 *  - arg4: 0
 *  - arg5: function address
 *  - arg6: address, pointed to 0
 *  - arg7: address, pointed to 0
 *  - arg8: 0x1c, 0x18, observation from FA: This value diff for scenario and name
 *  - arg9: 1
 *  - arg10: 0
 *  - arg11: 0 or 1, maybe, gender?
 *  - arg12: 0x00ffffff
 *  - arg13: addr
 *
 *  Guessed function signature:
 *  typedef int (__cdecl *hook_fun_t)(DWORD, DWORD, LPCSTR, DWORD, DWORD, DWORD,
 *                                    DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD);
 *  static int __cdecl newHookFun(DWORD arg1, DWORD arg2, LPCSTR str, DWORD arg4, DWORD arg5, DWORD arg6,
 *                                DWORD arg7, DWORD split, DWORD arg9, DWORD arg10, DWORD arg11, DWORD arg12, DWORD arg13)
 *
 *  Type 2: Sample game: 世界と世界の真ん中 体験版, relative address: 0x31850;
 *
 *  ? __cdecl sub_31850(DWORD arg1, DWORD arg2, LPCSTR arg3, DWORD arg4)
 *  - arg1: address, could point to area of zero, seems to be the output of this function
 *  - arg2: address, the same as arg1
 *  - arg3: string
 *  - arg4: flags, choices: 0, character name & scenario: 1
 *  - return: not sure, seems always to be 0 when success
 *
 *  Guessed function signature:
 *  - typedef int (__cdecl *hook_fun_t)(DWORD, DWORD, LPCSTR, DWORD); // __stdcall will crash the game
 *  - static int __cdecl newHookFun(DWORD arg1, DWORD arg2, LPCSTR str, DWORD split)
 */
void BGIEngine::hookFunction(HookStack *stack)
{
  static QByteArray data_; // persistent storage, which makes this function not thread-safe

  LPCSTR text3 = (LPCSTR)stack->args[2]; // arg3

  // In Type 1, split = arg8
  // In Type 2, there is no arg8. However, arg8 seems to be a good split that can differenciate choice and character name
  //DWORD split = stack->args[3]; // arg4
  DWORD split = stack->args[7]; // arg8

  auto sig = Engine::hashThreadSignature(stack->retaddr, split);

  data_ = instance()->dispatchTextA(text3, sig, Engine::UnknownRole);
  stack->args[2] = (DWORD)data_.constData();
}

// EOF
