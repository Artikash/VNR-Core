// circus.cc
// 6/5/2014 jichi
#include "engine/model/circus.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackfuns.h"
#include "hijack/hijackmanager.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include <qt_windows.h>

#define DEBUG "circus"
#include "sakurakit/skdebug.h"

/** Private data */

namespace { // unnamed

namespace ScenarioHook {

namespace Private {

  /**
   *  Sample game: DC3, function: 0x4201d0
   *
   *  IDA: sub_4201D0      proc near
   *  - arg_0 = dword ptr  4
   *  - arg_4 = dword ptr  8
   *
   *  Observations:
   *  - arg1: LPVOID, another function address
   *  - arg2: LPCSTR, the actual text
   */

  bool hookBefore(winhook::hook_stack *s)
  {
    static QByteArray data_; // persistent storage, which makes this function not thread-safe
    auto text = (LPCSTR)s->stack[2]; // arg2
    auto role = Engine::ScenarioRole;
    auto split = s->stack[0]; // retaddr
    auto sig = Engine::hashThreadSignature(role, split);
    data_ = EngineController::instance()->dispatchTextA(text, sig, role);
    s->stack[2] = (DWORD)data_.constData(); // reset arg2
    return true;
  }

} // namespace Private

/**
 *  jichi 6/5/2014: Sample function from DC3 at 0x4201d0
 *  004201ce     cc             int3
 *  004201cf     cc             int3
 *  004201d0  /$ 8b4c24 08      mov ecx,dword ptr ss:[esp+0x8]
 *  004201d4  |. 8a01           mov al,byte ptr ds:[ecx]
 *  004201d6  |. 84c0           test al,al
 *  004201d8  |. 74 1c          je short dc3.004201f6
 *  004201da  |. 8b5424 04      mov edx,dword ptr ss:[esp+0x4]
 *  004201de  |. 8bff           mov edi,edi
 *  004201e0  |> 3c 24          /cmp al,0x24
 *  004201e2  |. 75 05          |jnz short dc3.004201e9
 *  004201e4  |. 83c1 02        |add ecx,0x2
 *  004201e7  |. eb 04          |jmp short dc3.004201ed
 *  004201e9  |> 8802           |mov byte ptr ds:[edx],al
 *  004201eb  |. 42             |inc edx
 *  004201ec  |. 41             |inc ecx
 *  004201ed  |> 8a01           |mov al,byte ptr ds:[ecx]
 *  004201ef  |. 84c0           |test al,al
 *  004201f1  |.^75 ed          \jnz short dc3.004201e0
 *  004201f3  |. 8802           mov byte ptr ds:[edx],al
 *  004201f5  |. c3             retn
 *  004201f6  |> 8b4424 04      mov eax,dword ptr ss:[esp+0x4]
 *  004201fa  |. c600 00        mov byte ptr ds:[eax],0x0
 *  004201fd  \. c3             retn
 */
bool attach()
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;
  //return 0x4201d0; // DC3 function address
  for (ulong i = startAddress + 0x1000; i < stopAddress -4; i++)
    // *  004201e0  |> 3c 24          /cmp al,0x24
    // *  004201e2  |. 75 05          |jnz short dc3.004201e9
    if ((*(ulong *)i & 0xffffff) == 0x75243c) { // cmp al, 24; je
      enum { range = 0x80 }; // the range is small, since it is a small function
      if (ulong addr = MemDbg::findEnclosingAlignedFunction(i, range))
        return winhook::hook_before(addr, Private::hookBefore);
    }
  return false;
}

} // namespace ScenarioHook

} // unnamed namespace

/** Public class */

bool CircusEngine::attach()
{
  if (!ScenarioHook::attach())
    return false;
  HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineA);
  return true;
}

/**
 *  Get rid of ruby. Examples:
 *  ｛くらき／蔵木｝
 *  ｛・・・・／いいから｝この私に、紅茶を淹れなさい」
 */
QString CircusEngine::textFilter(const QString &text, int role)
{
  const wchar_t
    w_open = 0xff5b    /* ｛ */
    , w_close = 0xff5d /* ｝ */
    , w_split = 0xff0f /* ／ */
  ;
  if (role != Engine::ScenarioRole || !text.contains(w_open))
    return text;
  QString ret = text;
  for (int pos = ret.indexOf(w_open); pos != -1; pos = ret.indexOf(w_open, pos)) {
    int split_pos = ret.indexOf(w_split, pos);
    if (split_pos == -1)
      return ret;
    int close_pos = ret.indexOf(w_close, split_pos);
    if (close_pos == -1)
      return ret;
    ret.remove(close_pos, 1);
    ret.remove(pos, split_pos - pos + 1);
    pos += close_pos - split_pos - 1;
  }
  return ret;
}

// EOF

/*
// Remove "\n" for scenario text
QString CircusEngine::textFilter(const QString &text, int role)
{
  QString ret = text;
  if (role == Engine::ScenarioRole)
    ret.remove('\n');
  return ret;
}

// Insert "\n"
QString CircusEngine::translationFilter(const QString &text, int role)
{
  if (role != Engine::ScenarioRole)
    return text;
  enum { MaxLineWitdth = 61 }; // estimated in DC3. FIXME: This might be different for different game

  QString ret;
  int lineWidth = 0, // estimated line width
      lineCount = 0; // number of characters
  foreach (const QChar &c, text) {
    int w = c.unicode() <= 255 ? 1 : 2;
    if (lineWidth + w <= MaxLineWitdth) {
      lineCount ++;
      lineWidth += w;
    } else {
      bool wordWrap = false;
      if (lineCount)
        for (int i = 0; i < lineCount; i++)
          if (ret[ret.size() - i - 1].isSpace()) {
            wordWrap = true;
            ret[ret.size() - i - 1] = '\n';
            lineCount = i;
            lineWidth = 0;
            for (int i = 0; i < lineCount; i++)
              lineWidth +=  ret[ret.size() - i - 2].unicode() < 255 ? 1 : 2;
            break;
          }
      if (!wordWrap) {
        ret.append('\n');
        lineWidth = lineCount = 0;
      }
    }
    ret.append(c);
  }
  return ret;
}
*/
