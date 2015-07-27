// aoi.cc
// 6/6/2015 jichi
#include "engine/model/aoi.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include <qt_windows.h>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#define DEBUG "model/aoi"
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
  wstrT ltrimW(wstrT text)
  {
    static const char *quotes[] = { "<>", "[]" }; // skip leading quotes
    BOOST_FOREACH (const char *q, quotes)
      while (text[0] == q[0]) {
        if (auto p = ::wcschr(text, q[1])) {
          text = p + 1;
          if (*text == 0x3000) // skip \u3000 leading space
            text++;
        } else
          break;
      }
    return text;
  }

  /**
   *  Sample game:
   *  - 悪魔娘の看板料理: _AgsSpriteCreateText@20
   *  - BunnyBlack2: _AgsSpriteCreateText@24
   *
   *  - arg1: text address in heap, which can be directly overwritten
   *  - arg2: split
   *  - arg3:
   *  - arg4:
   *
   *  Sample text:
   *  [f9S30e0u]　が、それは人間相手の話だ。
   */
  bool beforeAgsSpriteCreateTextW(winhook::hook_stack *s)
  {
    // All threads including character names are linked together

    auto text = (LPWSTR)s->stack[1]; // arg1
    if (!text || !*text || !Engine::isAddressWritable(text)) // skip modifying readonly text in code region
      return true;

    bool containsTags = ::wcsstr(text, L"[u]");

    text = ltrimW(text);
    if (!*text)
      return true;

    int role = Engine::OtherRole;
    //ulong split = s->stack[0]; // retaddr
    ulong split = s->stack[2]; // arg2
    if (!containsTags)
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
            newText = EngineController::instance()->dispatchTextW(oldText, role, sig);
    if (newText == oldText)
      return true;
    text[newText.size()] = 0;
    if (!newText.isEmpty())
      newText.toWCharArray(text);
    return true;
  }

  bool beforeAgsSpriteCreateTextExW(winhook::hook_stack *s)
  {
    auto text = (LPWSTR)s->stack[2]; // arg2
    if (!text || !*text || !Engine::isAddressWritable(text))
      return true;

    text = ltrimW(text);
    if (!*text)
      return true;

    enum { role = Engine::OtherRole };
    ulong split = s->stack[0]; // retaddr
    auto sig = Engine::hashThreadSignature(role, split);

    QString oldText = QString::fromWCharArray(text),
            newText = EngineController::instance()->dispatchTextW(oldText, role, sig);
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
  if (!addr || !winhook::hook_before(addr, Private::beforeAgsSpriteCreateTextW))
    return false;
  if (addr = findCppProc(hModule, "AgsSpriteCreateTextEx", 1))
    winhook::hook_before(addr, Private::beforeAgsSpriteCreateTextExW);
  return true;
}

} // namespace AgsHookW

namespace AgsHookA {
namespace Private {

  template<typename wstrT>
  wstrT ltrimA(wstrT text)
  {
    static const char *quotes[] = { "<>", "[]" }; // skip leading quotes
    BOOST_FOREACH (const char *q, quotes)
      while (text[0] == q[0]) {
        if (auto p = ::strchr(text, q[1])) {
          text = p + 1;
          if ((uchar)text[0] == 0x81 && (uchar)text[1] == 0x40) // skip \u3000 leading space, assuming sjis encoding
            text += 2;
        } else
          break;
      }
    return text;
  }

  /**
   *  Sample game: 王賊
   *  _AgsSpriteCreateText@24 0x100108b0 0x000108b0 97 (0x61) Ags.dll Z:\Local\Windows\Games\SystemAoi\kingT\Ags.dll Exported Function
   *
   *  - arg1: text address in heap, which can be directly overwritten
   *  - arg2: split
   *  - arg3:
   *  - arg4:
   *
   *  Sample text:
   *  <s6e0u>八重
   *  <s6e0u>　ソフトホウスキャラ最新
   */
  bool beforeAgsSpriteCreateTextA(winhook::hook_stack *s)
  {
    // All threads including character names are linked together

    auto text = (LPSTR)s->stack[1]; // arg1
    if (!text || !*text || !Engine::isAddressWritable(text)) // skip modifying readonly text in code region
      return true;

    bool containsTags = ::strstr(text, "[u]");

    text = ltrimA(text);
    if (!*text)
      return true;

    int role = Engine::OtherRole;
    //ulong split = s->stack[0]; // retaddr
    ulong split = s->stack[2]; // arg2
    if (!containsTags)
      switch (split) {
      case 0x639d:
        role = Engine::NameRole;
        break;
      case 0x639c:
          role = Engine::ScenarioRole;
        break;
      }
    auto sig = Engine::hashThreadSignature(role, split);
    QByteArray data = EngineController::instance()->dispatchTextA(text, role, sig);
    ::strcpy(text, data.constData());
    return true;
  }

} // namespace Private

bool attach(HMODULE hModule) // attach scenario
{
  ulong addr = findCppProc(hModule, "AgsSpriteCreateText", 1);
  return addr && winhook::hook_before(addr, Private::beforeAgsSpriteCreateTextA);
}

} // namespace AgsHookA
} // unnamed namespace

/** Public class */

bool SystemAoiEngine::attach()
{
  HMODULE hModule = ::GetModuleHandleA("Ags.dll");
  if (hModule) { // Aoi <= 3
    if (!AgsHookA::attach(hModule))
      return false;
    name = "EmbedSystemAoiA";
    enableDynamicEncoding = true;
    HijackManager::instance()->attachFunction((ulong)::DrawTextExA); // Font can already be dynamically changed and hence not needed
    return true;

  } else { // Aoi >= 4, UTF-16
    hModule = ::GetModuleHandleA("Ags5.dll");
    if (!hModule)
      hModule = ::GetModuleHandleA("Ags4.dll");
    if (!hModule || !AgsHookW::attach(hModule))
      return false;
    name = "EmbedSystemAoiW";
    encoding = Utf16Encoding;
    return true;
  }
}

// EOF
