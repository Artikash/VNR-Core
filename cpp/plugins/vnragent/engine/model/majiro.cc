// majiro.cc
// 4/20/2014 jichi
// See: http://dev.haeleth.net/majiro.shtml
// See: http://bbs.sumisora.org/read.php?tid=10983263
// See: http://bbs.sumisora.org/read.php?tid=10917044
// See: http://bbs.sumisora.org/read.php?tid=225250
#include "engine/model/majiro.h"
#include "engine/enginedef.h"
#include "engine/engineenv.h"
#include "detoursutil/detoursutil.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>

//#define DEBUG "majiro"
#include "sakurakit/skdebug.h"

/** Private class */

// Make this as class instead of namespace so that it can be specified as friend
class MajiroEnginePrivate
{
  typedef MajiroEngine Q;

  // Return the number of bits in the integer
  // Brian-Kemighan's method
  // http://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer
  //static int bitCount(quint8 value)
  //{
  //  int count = 0;
  //  for (; value; count++)
  //    value &= value -1;
  //  return count;
  //}
  //static int bitCount(qint32 i)
  //static {
  //static   i = i - ((i >> 1) & 0x55555555);
  //static   i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
  //static   return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
  //static }

  /**
   *  Observeations from レミニセンス:
   *  - arg1 of the scenario is a fixed portable value.
   *  - arg2 of the scenario is not portable, but a constant for each run.
   *  - arg1 and arg2 2of both the name and other texts are random number.
   *  - arg2's first 4 bytes of name and scenario texts are the same.
   *  - arg4 is not portable, but a contant for each run.
   *  - arg5 is aways 1.
   *  - Scenario always comes after name
   *  - Scenario size always larger than (800,600), less than (1920,1080)
   *
   *  Game-specific arg1:
   *  - 暁の護衛 罪深き終末論: 32 = 0x20 = ' '
   *  - レミニセンス: 48 = 0x30 = '0'
   *  - PotentialAbility: 0xa0
   */
  static Engine::TextRole roleOf(BYTE arg1, LPSIZE arg2, LPVOID arg4, int arg5)
  {
    Q_UNUSED(arg4)
    Q_UNUSED(arg5)
    //static BYTE scenarioArg1_;
    static DWORD scenarioArg2_;

    enum { ScenarioMinWidth = 600, ScenarioMinHeight = 400 }; // always larger than 800, 800*3
    if (arg1 && !(arg1 & 0xf) && // lower 4 bits are zero
        arg2 && arg2->cx > ScenarioMinWidth && arg2->cy > ScenarioMinHeight) {
      scenarioArg2_ = (DWORD)arg2;
      return Engine::ScenarioRole;
    }
    enum { ScenarioMask = 0xffff0000 };
    if ((scenarioArg2_ & ScenarioMask) == ((DWORD)arg2 & ScenarioMask)) // the higher four bits of the scenario and the name are the same
      return Engine::NameRole;
    return Engine::OtherRole;
  }

public:

  /**
   *  Sample game: レミニセンス
   *  int __cdecl sub_41AF90(CHAR ch, int arg2, LPCSTR str, arg4, arg5);
   *  - ch: static, different for different kinds of text (chara, dialogue, etc.), this might be type ID
   *  - arg2: dynamic address, size of the text, different for different kinds of text
   *  - str: static address, the real text
   *  - arg4: dynamic, different for different scene (scenario, config, etc.), this might be an output value
   *  - arg5: static, always 1
   *  - return: width of the text, = 26 * text length
   *  Scenario text's ch seems to always be one.
   *
   *  Callers:
   *  0x41aa10 @ 0x416ab0 // Scenario
   *  0x41f650 // Name
   */
  static DWORD hookAddress;
  typedef int (__cdecl *hook_fun_t)(BYTE, LPSIZE, LPCSTR, LPVOID, int);
  static hook_fun_t oldHook;

  static int __cdecl newHook(BYTE arg1, LPSIZE arg2, LPCSTR str, LPVOID arg4, int arg5)
  {
    //return oldHook(arg1, arg2, str, arg4, arg5);
    auto q = static_cast<Q *>(AbstractEngine::instance());
    auto role = roleOf(arg1, arg2, arg4, arg5);
#ifdef DEBUG
    qDebug() << arg1 << ":" << arg2 << ":" << QString::fromLocal8Bit(str) << ":" << arg4 << ":" << arg5 << ";"
             << *((DWORD *)arg2) << "," << *((DWORD *)(arg2 + 1)) << ":" << *((DWORD *)arg4)
             << "=" << role << "," << arg2->cx << "," << arg2->cy;
#endif // DEBUG
    QByteArray data = q->dispatchTextA(str, role);
    if (!data.isEmpty())
      return oldHook(arg1, arg2, data, arg4, arg5);
    else {
      // Estimated return result
      enum { FontWidth = 26 };
      return FontWidth * ::strlen(str) * 2;
    }
  }
};
DWORD MajiroEnginePrivate::hookAddress;
MajiroEnginePrivate::hook_fun_t MajiroEnginePrivate::oldHook;

/** Public class */

bool MajiroEngine::match()
{ return Engine::glob(QStringList() << "data*.arc" << "stream*.arc"); }

bool MajiroEngine::inject()
{
  DWORD dwTextOutA = Engine::getModuleFunction("gdi32.dll", "TextOutA");
  if (!dwTextOutA)
    return false;
  DWORD startAddress, stopAddress;
  if (!Engine::getMemoryRange(nullptr, &startAddress, &stopAddress))
    return false;
  D::hookAddress = MemDbg::findCallerAddress(dwTextOutA, 0xec81, startAddress, stopAddress);
  // Note: ITH will mess up this value
  //D::hookAddress = 0x41af90; // レミニセンス function address
  if (!D::hookAddress)
    return false;
  D::oldHook = detours::replace<D::hook_fun_t>(D::hookAddress, D::newHook);
  //addr = 0x41f650; // 2
  //addr = 0x416ab0;
  //D::oldtest = detours::replace<D::test_fun_t>(addr, D::newtest);
  return true;
}

bool MajiroEngine::unload()
{
  if (!D::hookAddress || !D::oldHook)
    return false;
  bool ok = detours::restore<D::hook_fun_t>(D::hookAddress, D::oldHook);
  D::hookAddress = 0;
  return ok;
}

// EOF

/*
void MajiroEngine::drawText(const QString &text, const void *context)
{
  Q_ASSERT(context);
  auto params = static_cast<const D::Context *>(context);
  D::olddraw(params->arg1, params->arg2, text.toLocal8Bit(), params->arg4, params->arg5);
}

void MajiroEngine::releaseContext(void *context)
{
  Q_ASSERT(context);
  delete static_cast<D::Context *>(context);
}
*/
