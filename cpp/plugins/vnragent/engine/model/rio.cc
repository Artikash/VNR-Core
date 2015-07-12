// rio.cc
// 7/10/2015 jichi
//
// 椎名里緒 versions: http://green.ribbon.to/~erog/Note022.html
//
// Engine protected with Themida:
// - Game abort when OllyDbg is openned
// - Game module memory space protected that cannot be modified
//
// Debugging method:
// - Use OllyDbg + PhantOm: https://tuts4you.com/download.php?view.1276
// - Rename OllyDbg.exe to anything other than OllyDbg.exe
// - Hook to GDI functions and then check memory stack
#include "engine/model/rio.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include <qt_windows.h>
#include <QtCore/QRegExp>
#include <climits>
#include <fstream>
#include <unordered_set>

#define DEBUG "model/rio"
#include "sakurakit/skdebug.h"

/** Private data */

namespace { // unnamed

/**
 * Sample first line in RIO.INI:
 * [椎名里緒 v2.50]
 * return 250 (major 2, minor 50)
 */
int getRioVersion()
{
  std::string line;
  { // get first line
    std::ifstream f("RIO.INI");
    if (f.is_open()) {
      std::getline(f, line);
      f.close();
    }
  }
  enum { TagSize = 8 }; // size of ShinaRio
  if (line.size() >= TagSize + 8 && line[0] == '[' && line[TagSize + 1] == ' ' && line[TagSize + 2] == 'v' && line[TagSize + 4] == '.') {
    char major = line[TagSize + 3],
         minor1 = line[TagSize + 5],
         minor2 = line[TagSize + 6];
    if (::isdigit(major)) {
      int ret = (major - '0') * 100;
      if (::isdigit(minor1))
        ret += (minor1 - '0') * 10;
      if (::isdigit(minor2))
        ret += (minor2 - '0') * 1;
      return ret;
    }
  }
  return 0;
}

namespace ScenarioHook {
namespace Private {

  class HookArgument
  {
    DWORD split,
          offset[0x57]; // [esi]+0x160
    LPSTR text;   // [esi]+0x160

    template <typename strT>
    static strT nextText(strT t)
    {
      t += ::strlen(t);
      return (t[6] && !t[5] && !t[4] && !t[3] && !t[2] && !t[1]) ? t + 6 : nullptr; // 6 continuous zeros
    }

    bool isList() const { return nextText(text); }

    Engine::TextRole textRole() const
    {
      static ulong minSplit_ = UINT_MAX;
      minSplit_ = qMin(minSplit_, split);
      return split == minSplit_ ? Engine::ScenarioRole :
             split == minSplit_ + 1 ? Engine::NameRole :
             Engine::OtherRole;
    }

    void dispatchText()
    {
      enum { NameCapacity = 0x10 }; // including ending '\0'
      static QByteArray data_;

      if (0 == ::strcmp(text, data_.constData()))
        return;

      //LPSIZE lpSize = (LPSIZE)s->stack[4]; // arg4 of GetTextExtentPoint32A
      //int area = lpSize->cx * lpSize->cy;
      //auto role = lpSize->cx || !lpSize->cy || area > 150 ? Engine::ScenarioRole : Engine::NameRole;
      auto role = textRole();
      auto sig = Engine::hashThreadSignature(role, split);
      QByteArray oldData = text,
                 newData = EngineController::instance()->dispatchTextA(oldData, role, sig);
      if (newData == oldData)
        return;
      data_ = newData;

      if (role == Engine::NameRole && newData.size() >= NameCapacity) {
        data_ = newData.left(NameCapacity - 1);
        ::strncpy(text, newData.constData(), NameCapacity);
        text[NameCapacity] = 0;
      } else {
        ::strcpy(text, newData.constData());
        if (oldData.size() > newData.size())
          ::memset(text + newData.size(), 0, oldData.size() - newData.size());
      }
    }

    void dispatchList()
    {
      static std::unordered_set<qint64> hashes_;
      enum { role = Engine::OtherRole };
      auto sig = Engine::hashThreadSignature(role, split);
      for (auto p = text; p; p = nextText(p)) {
        if (hashes_.find(Engine::hashCharArray(p)) != hashes_.end())
          continue;
        QByteArray oldData = p,
                   newData = EngineController::instance()->dispatchTextA(oldData, role, sig);
        if (newData != oldData) {
          if (newData.size() > oldData.size())
            newData = newData.left(oldData.size());
          else
            while (newData.size() < oldData.size())
              newData.push_back(' ');
          ::memcpy(p, newData.constData(), oldData.size());
          hashes_.insert(Engine::hashByteArray(newData));
        }
      }
    }

  public:
    bool isValid() const { return Engine::isAddressWritable(text) && *text; }
    void dispatch() { if (isList()) dispatchList(); else dispatchText(); }
  };

  /**
   *
   *  BOOL GetTextExtentPoint32(HDC hdc, LPCTSTR lpString, int c, LPSIZE lpSize);
   *
   *  Scenario:
   *  0012F4EC   0043784C  /CALL to GetTextExtentPoint32A from .00437846
   *  0012F4F0   9A010C64  |hDC = 9A010C64
   *  0012F4F4   004C0F30  |Text = "Y"
   *  0012F4F8   00000001  |TextLen = 0x1
   *  0012F4FC   00504DA4  \pSize = .00504DA4
   *  0012F500   00503778  .00503778
   *  0012F504   00439EBE  RETURN to .00439EBE from .00437790
   *  0012F508   00503778  .00503778
   *  0012F50C   00914CC0  .00914CC0
   *  0012F510   00000001
   *  0012F514   00503778  .00503778
   *  0012F518   0069EB80  .0069EB80
   *  0012F51C   00000000
   *  0012F520   00914CC0  .00914CC0
   *  0012F524   0600A0AE
   *  0012F528   0012F53C  ASCII "ps"
   *  0012F52C   76DD23CB  user32.ClientToScreen
   *  0012F530   75D0BA46  kernel32.Sleep
   *
   *  pSize:
   *  00504DA4  0C 00 00 00 18 00 00 00 18 00 00 00 15 00 00 00  .............
   *  00504DB4  03 00 00 00 00 00 00 00 00 00 00 00 0C 00 00 00  ...............
   *  00504DC4  1B 00 00 00 90 01 00 00 00 00 00 00 60 00 00 00  ...・......`...
   *  00504DD4  60 00 00 00 00 FF A5 02 00 00 00 36 80 00 00 00  `....･...6...
   *  00504DE4  01 00 00 00 00 00 00 00 00 00 00 00 0D 00 00 00  ...............
   *  00504DF4  00 00 00 00 00 00 00 00 00 00 00 00 64 00 00 00  ............d...
   *  00504E04  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504E14  82 6C 82 72 20 83 53 83 56 83 62 83 4E 00 00 00  ＭＳ ゴシック...
   *  00504E24  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *
   *  Name:
   *  0012F4EC   0043784C  /CALL to GetTextExtentPoint32A from .00437846
   *  0012F4F0   9A010C64  |hDC = 9A010C64
   *  0012F4F4   004C0F30  |Text = "Y"
   *  0012F4F8   00000001  |TextLen = 0x1
   *  0012F4FC   00506410  \pSize = .00506410
   *  0012F500   00504DE4  .00504DE4
   *  0012F504   00439EBE  RETURN to .00439EBE from .00437790
   *  0012F508   00504DE4  .00504DE4
   *  0012F50C   00914CC0  .00914CC0
   *  0012F510   00000001
   *  0012F514   00504DE4  .00504DE4
   *  0012F518   006A1868  .006A1868
   *  0012F51C   00000000
   *  0012F520   00914CC0  .00914CC0
   *
   *  pSize:
   *  00506410  07 00 00 00 0D 00 00 00 0D 00 00 00 0B 00 00 00  ..............
   *  00506420  02 00 00 00 00 00 00 00 00 00 00 00 07 00 00 00  ..............
   *  00506430  0F 00 00 00 90 01 00 00 00 00 00 00 60 00 00 00  ...・......`...
   *  00506440  60 00 00 00 00 FF A5 02 00 00 00 36 80 00 00 00  `....･...6...
   *  00506450  02 00 00 00 00 00 00 00 00 00 00 00 18 00 00 00  ..............
   *  00506460  00 00 00 00 00 00 00 00 00 00 00 00 64 00 00 00  ............d...
   *  00506470  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00506480  82 6C 82 72 20 83 53 83 56 83 62 83 4E 00 00 00  ＭＳ ゴシック...
   *  00506490  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *
   *  Values of esi:
   *
   *  Name:
   *  00504DE4  01 00 00 00 B6 0C 0A 76 02 00 00 00 0D 00 00 00  ...ｶ..v.......
   *  00504DF4  00 00 00 00 00 00 00 00 00 00 00 00 64 00 00 00  ............d...
   *  00504E04  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504E14  82 6C 82 72 20 83 53 83 56 83 62 83 4E 00 00 00  ＭＳ ゴシック...
   *  00504E24  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504E34  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504E44  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504E54  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504E64  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504E74  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  ...
   *  00504F44  7C 78 FF 05 3E 00 00 00 3E 00 00 00 02 00 00 00  |x>...>......
   *  00504F54  3E 00 00 00 02 00 00 00 06 00 00 00 00 00 00 00  >.............
   *  00504F64  0C 00 00 00 00 00 00 00 01 00 00 00 31 D9 D3 00  ...........1ﾙﾓ.
   *  00504F74  00 00 00 00 00 00 00 00 00 00 00 00 00 05 00 00  ...............
   *
   *  00504DE4  01 00 00 00 35 06 0A 89 02 00 00 00 0D 00 00 00  ...5.・.......
   *  00504DF4  00 00 00 00 00 00 00 00 00 00 00 00 64 00 00 00  ............d...
   *  00504E04  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504E14  82 6C 82 72 20 83 53 83 56 83 62 83 4E 00 00 00  ＭＳ ゴシック...
   *  00504E24  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504E34  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504E44  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504E54  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504E64  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504E74  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504E84  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504E94  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504EA4  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504EB4  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504EC4  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504ED4  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504EE4  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504EF4  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504F04  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00504F14  FF FF FF FF 01 00 00 00 00 01 00 00 00 01 00 00  .........
   *  00504F24  00 01 00 00 FF FF FF 00 00 00 00 00 00 00 00 00  ............
   *  00504F34  01 00 00 00 01 00 00 00 01 00 00 00 01 00 00 00  ............
   *  00504F44  7C 78 0C 06 3E 00 00 00 3E 00 00 00 02 00 00 00  |x.>...>......
   *  00504F54  3E 00 00 00 02 00 00 00 06 00 00 00 00 00 00 00  >.............
   *  00504F64  0C 00 00 00 00 00 00 00 01 00 00 00 C3 46 04 01  ...........ﾃF
   *  00504F74  00 00 00 00 00 00 00 00 00 00 00 00 00 05 00 00  ...............
   *  00504F84  00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00  ...............
   *  00504F94  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *
   *  Scenario:
   *  00503778  00 00 00 00 99 12 0A 24 02 00 00 00 18 00 00 00  ....・.$......
   *  00503788  00 00 00 00 00 00 00 00 00 00 00 00 64 00 00 00  ............d...
   *  00503798  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  005037A8  82 6C 82 72 20 83 53 83 56 83 62 83 4E 00 00 00  ＭＳ ゴシック...
   *  005037B8  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  ...
   *  005038D8  7C 70 0C 06 24 01 00 00 24 01 00 00 17 00 00 00  |p.$..$.....
   *  005038E8  24 01 00 00 17 00 00 00 0C 00 00 00 2A 00 00 00  $.........*...
   *  005038F8  18 00 00 00 00 00 00 00 01 00 00 00 6D C6 05 01  ..........mﾆ
   *  00503908  00 00 00 00 00 00 00 00 00 00 00 00 18 04 00 00  ..............
   *  00503918  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00503928  0D 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00503938  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00503948  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *
   *  00503778  00 00 00 00 40 12 0A 9A 02 00 00 00 18 00 00 00  ....@.・......
   *  00503788  00 00 00 00 00 00 00 00 00 00 00 00 64 00 00 00  ............d...
   *  00503798  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  005037A8  82 6C 82 72 20 83 53 83 56 83 62 83 4E 00 00 00  ＭＳ ゴシック...
   *  005037B8  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  005037C8  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  005037D8  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  005037E8  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  005037F8  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00503808  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00503818  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00503828  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00503838  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00503848  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00503858  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00503868  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00503878  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00503888  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  00503898  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  005038A8  FF FF FF FF 01 00 00 00 00 01 00 00 00 01 00 00  .........
   *  005038B8  00 01 00 00 FF FF FF 00 00 00 00 00 00 00 00 00  ............
   *  005038C8  01 00 00 00 01 00 00 00 01 00 00 00 01 00 00 00  ............
   *  005038D8  7C 70 0C 06 E4 01 00 00 E4 01 00 00 2C 00 00 00  |p.・..・..,...
   *  005038E8  E4 01 00 00 2C 00 00 00 0C 00 00 00 2A 00 00 00  ・..,.......*...
   *  005038F8  18 00 00 00 00 00 00 00 01 00 00 00 5A F5 11 01  ..........Z・
   *  00503908  00 00 00 00 00 00 00 00 00 00 00 00 18 04 00 00  ..............
   *  00503918  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   */
  bool hookBefore(winhook::hook_stack *s)
  {
    auto arg = (HookArgument *)s->esi;
    if (arg->isValid())
      arg->dispatch();
    return true;
  }

} // namespace Private

/**
 *  Sample game: 幻創のイデア (RIO 2.49)
 *  Text painted by GetGlyphOutlineA.
 *  Debugged by attaching to GetTextExtentPoint32A.
 *  There is only one GetTextExtentPoint32A in the game, where only 'Y' (0x59) is calculated.
 *  Text is in a large memory region that can be modified.
 *
 *  When the text contains new line (_r), the same text will be invoked twice.
 *  Need to avoid immediate duplicate.
 */
bool attach()
{ return winhook::hook_before((ulong)::GetTextExtentPoint32A, Private::hookBefore); }

} // namespace ScenarioHook
} // unnamed namespace

/** Public class */

bool ShinaRioEngine::attach()
{
  int version = ::getRioVersion();
  DOUT("version =" << version);
  if (version < 248) // currently only >= 2.48 is supported
    return false;
  if (!ScenarioHook::attach())
    return false;

  HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineA);
  return true;
}

/**
 *  Remove dots
 *  Sample sentence:
 *  もう一つは、この事実を受けて自分はどうするべきなのか――正確には、_t!250,6,6,・・・・・・・/どうしたいのかという決断に直面したからだった。
 */

QString ShinaRioEngine::rubyRemove(const QString &text)
{
  if (!text.contains("_t")) //role != Engine::ScenarioRole ||
    return text;
  static QRegExp rx("_t.*/");
  if (!rx.isMinimal())
    rx.setMinimal(true);
  return QString(text).remove(rx);
}

// FIXME: Ruby creation rule does not work
QString ShinaRioEngine::rubyCreate(const QString &rb, const QString &rt)
{
  static QString fmt("_t!250,6,6,%2/%1");
  return fmt.arg(rb, rt);
}

// EOF
