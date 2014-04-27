// majiro.cc
// 4/20/2014 jichi
// See also: http://bbs.sumisora.org/read.php?tid=10983263

#include "model/engine/majiro.h"
#include "model/env.h"
#include "detoursutil/detoursutil.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>
#include <QtCore/QStringList>
//#include <QtCore/QDebug>

/**
 * レミニセンス:
 * int __cdecl sub_41AF90(CHAR ch, int arg2, LPCSTR str, arg4, arg5);
 * - ch: static, different for different kinds of text (chara, dialogue, etc.), this might be type ID
 * - arg2: dynamic, different for different kinds of text, this might be texture ID
 * - str: static, the real text
 * - arg4: dynamci, different for different scene (scenario, config, etc.), this might be deviceId
 * - arg5: static, always 1
 * Scenario text's ch seems to always be one.
 */
typedef int (* paint_fun_t)(char, int, const char *, int, int);
static paint_fun_t oldpaint;

//static int newpaint(char ch, int arg2, const char *str, int arg4, int arg5)
//qDebug() << ch << ":" << arg2 << ":" << QString::fromLocal8Bit(str) << ":" << arg4 << ":" << arg5;
static int newpaint(char type, int textureId, const char *str, int deviceId, int flags)
{
  QString t = AbstractEngine::instance()->translate(str);
  if (t.isEmpty())
    return ::oldpaint(type, arg2, str, arg4, flags);
  else
    return ::oldpaint(ch, arg2, t.toLocal8Bit(), arg4, flags);
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
