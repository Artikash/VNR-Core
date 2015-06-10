// wolf.cc
// 6/9/2015 jichi
#include "engine/model/wolf.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include <qt_windows.h>
//#include <QtCore/QRegExp>
//#include <QtCore/QSet>

#define DEBUG "wolf"
#include "sakurakit/skdebug.h"

//#pragma intrinsic(_ReturnAddress)

namespace { // unnamed

namespace ScenarioHook {

namespace Private {

  struct FunctionStack
  {
    ulong retaddr;
    LPSTR text; // arg1
    ulong arg2;
    ulong arg3;
  };

  bool hookBefore(winhook::hook_stack *s)
  {
    static QByteArray data_;
    enum { role = Engine::ScenarioRole };
    auto fs = (FunctionStack *)s->stack;

    if (fs->text && (quint8)*fs->text > 127) {
      auto sig = Engine::hashThreadSignature(role, fs->retaddr);

      QByteArray oldData = fs->text;
      QByteArray newData = EngineController::instance()->dispatchTextA(oldData, sig, role);
      if (newData != oldData) {
        if (data_ != oldData)
          data_ = oldData;
        fs->text = (LPSTR)data_.constData();
        //lastData_ = newData;
        //if (newData.size() < oldData.size())
        //  ::memset(fs->text + newData.size(), 0, oldData.size() - newData.size());
        //::strcpy(fs->text, newData.constData());
      }
    }
    return true;
  }

} // namespace Private

/**
 *  Sample game: DRAGON SLAVE
 *
 *  004cd3ad   cc               int3
 *  004cd3ae   cc               int3
 *  004cd3af   cc               int3
 *  004cd3b0   8b4424 0c        mov eax,dword ptr ss:[esp+0xc]	; jichi: text in arg1
 *  004cd3b4   53               push ebx
 *  004cd3b5   85c0             test eax,eax
 *  004cd3b7   74 52            je short .004cd40b
 *
 *  It seems the function takes three arguments, and text in arg1
 *  00126708   004A5917  RETURN to .004A5917 from .004CD3B0
 *  0012670C   08A6F978 ; jichi: text here
 *  00126710   00000040
 *  00126714   0000003B
 *  00126718  /0012673C
 *  0012671C  |004A5892  RETURN to .004A5892 from .004A5900
 *  00126720  |08A6F978
 *  00126724  |0000003B
 *  00126728  |0069A6C8  .0069A6C8
 *  0012672C  |001278E8
 *  00126730  |08A6F978
 *  00126734  |77DAFFCF  RETURN to ntdll.77DAFFCF from ntdll.77DAFFF1
 */
bool attach() // attach other text
{
  ulong addr = 0x004cd3b0;
  return winhook::hook_before(addr, Private::hookBefore);
}

} // namespace ScenarioHook

} // unnamed namespace

/** Public class */

bool WolfRPGEngine::attach()
{
  if (!ScenarioHook::attach())
    return false;

  HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineA); // for dynamic encoding
  return true;
}

// EOF
