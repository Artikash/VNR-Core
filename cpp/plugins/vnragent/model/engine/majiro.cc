// majiro.cc
// 4/20/2014 jichi
// See also: http://bbs.sumisora.org/read.php?tid=10983263

#include "model/engine/majiro.h"
#include "model/env.h"
#include "engine/enginedef.h"
#include "detoursutil/detoursutil.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>
#include <QtCore/QStringList>
#include <QtCore/QDebug>

/** Private class */

// Make this as class instead of namespace so that it can be specified as friend
class MajiroEnginePrivate
{
  typedef MajiroEngine Q;

  static Engine::TextRole roleOf(char arg1, int arg2, int arg4, int arg5)
  {
    Q_UNUSED(arg2)
    Q_UNUSED(arg4)
    Q_UNUSED(arg5)
    switch (arg1) {
    case 0: return Engine::NameRole;
    //case 1: return Engine::NameRole;
    default: return Engine::ScenarioRole;
    }
  }

public:

  /**
   *  Sample game: レミニセンス:
   *  int __cdecl sub_41AF90(CHAR ch, int arg2, LPCSTR str, arg4, arg5);
   *  - ch: static, different for different kinds of text (chara, dialogue, etc.), this might be type ID
   *  - arg2: dynamic, different for different kinds of text, this might be texture ID
   *  - str: static, the real text
   *  - arg4: dynamci, different for different scene (scenario, config, etc.), this might be deviceId
   *  - arg5: static, always 1
   *  - return: width of the text, = 26 * text length
   *  Scenario text's ch seems to always be one.
   *
   *  Callers:
   *  0x41aa10 @ 0x416ab0 // Scenario
   *  0x41f650 // Name
   */
  typedef int (* draw_fun_t)(char, int, const char *, int, int);
  static draw_fun_t olddraw;

  static int newdraw(char arg1, int arg2, const char *str, int arg4, int arg5)
  {
    //qDebug() << arg1 << ":" << arg2 << ":" << QString::fromLocal8Bit(str) << ":" << arg4 << ":" << arg5;
    //return olddraw(arg1, arg2, str, arg4, arg5);
    auto q = static_cast<Q *>(AbstractEngine::instance());
    auto role = roleOf(arg1, arg2, arg4, arg5);
    QByteArray data = str;
    QString t = q->dispatchText(data, role);
    if (!t.isEmpty())
      return olddraw(arg1, arg2, t.toLocal8Bit(), arg4, arg5);

    return olddraw(arg1, arg2, str, arg4, arg5);

    // Estimated return result
    enum { FontWidth = 26 };
    return FontWidth * data.size() * 2;
  }
};
MajiroEnginePrivate::draw_fun_t MajiroEnginePrivate::olddraw;

/** Public class */

bool MajiroEngine::match()
{ return Env::glob(QStringList() << "data*.arc" << "stream*.arc"); }

bool MajiroEngine::inject()
{
  DWORD dwTextOutA = Env::getModuleFunction("gdi32.dll", "TextOutA");
  if (!dwTextOutA)
    return false;
  DWORD startAddress, stopAddress;
  if (!Env::getMemoryRange(nullptr, &startAddress, &stopAddress))
    return false;
  DWORD addr = MemDbg::findCallerAddress(dwTextOutA, 0xec81, startAddress, stopAddress);
  // Note: ITH will mess up this value
  addr = 0x41af90;
  if (!addr)
    return false;
  D::olddraw = detours::replace<D::draw_fun_t>(addr, D::newdraw);
  //addr = 0x41f650; // 2
  //addr = 0x416ab0;
  //D::oldtest = detours::replace<D::test_fun_t>(addr, D::newtest);
  return addr;
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
