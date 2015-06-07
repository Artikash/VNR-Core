// rgss.cc
// 6/7/2015 jichi
#include "engine/model/rgss.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackfuns.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include <qt_windows.h>
#include <boost/foreach.hpp>

#define DEBUG "age"
#include "sakurakit/skdebug.h"

#pragma intrinsic(_ReturnAddress)

namespace { // unnamed

namespace RGSS3Hook {
namespace Private {

  struct HookArguments
  {
    char *dst; // arg1
    const char *src; // arg2
    size_t size; // arg3
  };
  bool hookBefore(winhook::hook_stack *s)
  {
    auto args = (HookArguments *)(s->stack);
    if (args->size) {
      QString oldText = QString::fromUtf8(args->src, args->size);
      if (oldText[0].unicode() >= 128) {

        enum { role = Engine::OtherRole };
        ulong split = (ulong)_ReturnAddress();
        auto sig = Engine::hashThreadSignature(role, split);

        QString newText = EngineController::instance()->dispatchTextW(oldText, sig, role);
        if (newText != oldText) {
          static QByteArray data;
          data = newText.toUtf8();
          s->eax = args->size = data.size();
          s->edx = ulong(args->src = data.constData());
        }
      }
    }
    return true;
  }

} // namespace Private

// 1004154C   8D4B 08          LEA ECX,DWORD PTR DS:[EBX+0x8]
// 1004154F   74 03            JE SHORT RGSS301.10041554
// 10041551   8B4B 0C          MOV ECX,DWORD PTR DS:[EBX+0xC]
// 10041554   50               PUSH EAX ; jichi: arg3 size
// 10041555   52               PUSH EDX ; jichi: arg2 src
// 10041556   51               PUSH ECX ; jichi: arg1 dst
// 10041557   E8 E4F21300      CALL RGSS301.10180840
// 1004155C   8B5424 24        MOV EDX,DWORD PTR SS:[ESP+0x24]
// 10041560   8B02             MOV EAX,DWORD PTR DS:[EDX]
// 10041562   8BC8             MOV ECX,EAX
// 10041564   83C4 0C          ADD ESP,0xC

bool attach(HMODULE hModule) // attach scenario
{
  ulong addr = 0x10041557;
  return addr && winhook::hook_before(addr, Private::hookBefore);
}

} // namespace AgsHookW
} // unnamed namespace

/** Public class */

bool RGSSEngine::attach()
{
  HMODULE hModule = ::GetModuleHandleA("RGSS301.dll");
  if (!hModule)
    return false;
  return RGSS3Hook::attach(hModule);
}

// EOF
/*
  //typedef char *(* hook_fun_t)(char *dst, const char *src, size_t size);
  //hook_fun_t oldHookFun;
  char *oldHookFun(char *dst, const char *src, size_t size)
  {
    ::memcpy(dst, src, size);
    if (src[size] == 0)
      dst[size] = 0;
    return dst;
  }

  // Similar to strncpy
  // Copy size of src to dst and append '\0' at the end
  // Return dst
  char *newHookFun(char *dst, const char *src, size_t size)
  {
    if (size) {
      QString oldText = QString::fromUtf8(src, size);
      if (oldText[0].unicode() >= 128) {

        enum { role = Engine::OtherRole };
        ulong split = (ulong)_ReturnAddress();
        auto sig = Engine::hashThreadSignature(role, split);

        QString newText = EngineController::instance()->dispatchTextW(oldText, sig, role);
        if (newText != oldText) {
          static QByteArray data;
          data = newText.toUtf8();
          return oldHookFun(dst, data.constData(), data.size());
        }
      }
    }
    return oldHookFun(dst, src, size);
  }
*/
