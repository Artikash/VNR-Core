// majiro.cc
// 4/20/2014 jichi
// See also: http://bbs.sumisora.org/read.php?tid=10983263

#include "model/engine/majiro.h"
#include "model/env.h"
#include "detoursutil/detoursutil.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>
#include <QtCore/QStringList>
#include "debug.h"

// レミニセンス:
// int __cdecl sub_41AF90(CHAR String, int, LPCSTR lpString, int, int);
typedef int (* paint_fun_t)(char, int, const char *, int, int);
static paint_fun_t oldpaint;

static int newpaint(char ch, int arg2, const char *str, int arg4, int arg5)
{
  //dmsg(QString("%1:%2: %3:%4:%5 :%6:%7")
  //  .arg(QString(QChar(ch)))
  //  .arg(QString::number(arg2))
  //  .arg(QString::fromLocal8Bit(str))
  //    .arg(QString::fromLocal8Bit(str).size())
  //    .arg(QString::number(qstrlen(str)))
  //  .arg(QString::number(arg4))
  //  .arg(QString::number(arg5))
  //);
  QString t = AbstractEngine::instance()->translate(str);
  if (t.isEmpty())
    return ::oldpaint(ch, arg2, str, arg4, arg5);
  else
    return ::oldpaint(ch, arg2, t.toLocal8Bit(), arg4, arg5);
}

bool MajiroEngine::match() { return Env::glob(QStringList() << "data*.arc" << "stream*.arc"); }

bool MajiroEngine::inject()
{
  DWORD startAddress, stopAddress;
  if (!Env::getMemoryRange(nullptr, &startAddress, &stopAddress))
    return false;
  DWORD addr = MemDbg::findCallerAddress((DWORD)TextOutA, 0xec81, startAddress, stopAddress);
  // Note: ITH will mess up this value
  addr = 0x41af90;
  if (!addr)
    return false;
  ::oldpaint = detours::replace<paint_fun_t>(addr, ::newpaint);
  return addr;
}

// EOF
