// aoi.cc
// 6/6/2015 jichi
#include "engine/model/aoi.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackfuns.h"
#include "hijack/hijackmanager.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include <qt_windows.h>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#define DEBUG "age"
#include "sakurakit/skdebug.h"

namespace { // unnamed

// Helper functions

/**
 * @param  hModule
 * @param  functionName  original function name without name hashing
 * @param  minParamNum  minimum number of parameters
 * @param  maxParamNum  maximum number of parameters
 * @return  address of the procedure or 0
 */
ulong findCppProc(HMODULE hModule, LPCSTR functionName, int minParamNum = 0, int maxParamNum = 10)
{
  for (int i = minParamNum; i < maxParamNum; i++) {
    std::string sig; // function signature name, such as _AgsSpriteCreateText@20
    sig.push_back('_');
    sig += functionName;
    sig.push_back('@');
    sig += boost::lexical_cast<std::string>(4 * i);
    if (auto proc = ::GetProcAddress(hModule, sig.c_str()))
      return (ulong)proc;
  }
  return 0;
}

namespace AgsHookW {
namespace Private {

  template<typename wstrT>
  wstrT ltrimTextW(wstrT text)
  {
    static const char *quotes[] = { "<>", "[]" }; // skip leading quotes
    BOOST_FOREACH (const char *q, quotes) {
      if (text[0] == q[0]) {
        if (auto p = ::wcschr(text, q[1])) {
          text = p + 1;
          if (*text == 0x3000) // skip \u3000 leading space
            text++;
        }
        break;
      }
    }
    return text;
  }

  /**
   *  Sample game:
   *  - 悪魔娘の看板料理: _AgsSpriteCreateText@20
   *  - BunnyBlack2: _AgsSpriteCreateText@24
   *
   *  - arg1: text address in heap, which can be directly overwritten
   *  - arg2:
   *  - arg3:
   *  - arg4:
   *
   *  Sample text:
   *  [f9S30e0u]　が、それは人間相手の話だ。
   */
  bool beforeAgsSpriteCreateText(winhook::hook_stack *s)
  {
    // All threads including character names are linked together

    LPWSTR text = (LPWSTR)s->stack[1]; // arg1
    if (!text || !*text || !Engine::isAddressWritable(text)) // skip modifying readonly text in code region
      return true;

    text = ltrimTextW(text);
    if (!*text)
      return true;

    int role = Engine::OtherRole;
    //ulong split = s->stack[0]; // retaddr
    ulong split = s->stack[2]; // arg2
    switch (split) {
    case 0x63a1:
      role = Engine::NameRole;
      break;
    case 0x639e:
      role = Engine::ScenarioRole;
      break;
    }
    auto sig = Engine::hashThreadSignature(role, split);

    QString oldText = QString::fromWCharArray(text),
            newText = EngineController::instance()->dispatchTextW(oldText, sig, role);
    if (newText == oldText)
      return true;
    text[newText.size()] = 0;
    if (!newText.isEmpty())
      newText.toWCharArray(text);
    return true;
  }

} // namespace Private

bool attach(HMODULE hModule) // attach scenario
{
  ulong addr = findCppProc(hModule, "AgsSpriteCreateText", 1);
  if (!addr || !winhook::hook_before(addr, Private::beforeAgsSpriteCreateText))
    return false;

  //if (addr = findCppProc(hModule, "AgsSpriteCreateTextEx", 1))
  //  winhook::hook_before(addr, Private::beforeAgsSpriteCreateTextEx);
  return true;
}

} // namespace AgsHookW
} // unnamed namespace

/** Public class */

bool SystemAoiWEngine::attach()
{
  HMODULE hModule = ::GetModuleHandleA("Ags5.dll");
  if (!hModule)
    hModule = ::GetModuleHandleA("Ags4.dll");
  if (!hModule)
    return false;
  return AgsHookW::attach(hModule);
}

// EOF

/*
  bool beforeAgsSpriteCreateTextEx(winhook::hook_stack *s)
  {
    static QString text_;
    // All threads including character names are linked together

    LPCWSTR text = (LPCWSTR)s->stack[1]; // arg1
    if (!text || !*text)
      return true;

    LPCWSTR trimmedText = ltrimTextW(text);
    if (!*trimmedText)
      return true;

    enum { role = Engine::OtherRole };
    ulong split = s->stack[0]; // retaddr
    auto sig = Engine::hashThreadSignature(role, split);

    QString oldText = QString::fromWCharArray(trimmedText),
            newText = EngineController::instance()->dispatchTextW(oldText, sig, role);
    if (newText == oldText)
      return true;
    text_ = newText;
    if (text != trimmedText) {
      QString prefix = QString::fromWCharArray(text, trimmedText - text);
      text_.prepend(prefix);
    }
    s->stack[1] = (ulong)text_.utf16(); // arg1
    return true;
  }
*/
