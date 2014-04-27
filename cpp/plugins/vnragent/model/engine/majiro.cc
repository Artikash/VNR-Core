// majiro.cc
// 4/20/2014 jichi
// See also: http://bbs.sumisora.org/read.php?tid=10983263

#include "model/engine/majiro.h"
#include "detoursutil/detoursutil.h"
#include "memdbg/memsearch.h"
#include "ntinspect/ntinspect.h"
#include <QtCore/QStringList>

namespace majiro {

// int __cdecl sub_41AF90(CHAR String, int, LPCSTR lpString, int, int);
typedef int (* paint_func_t)(char, int, LPCSTR, int, int);
paint_func_t paint;
int mypaint(char ch, int x, LPCSTR str, int y, int z)
{
  //growl::debug("majiro::paint");
  if (!paint)
    return 0;
  str = "hello world";
  return paint(ch, x, str, y, z);
}

} // majiro

bool MajiroEngine::match() { return glob(QStringList() << "data*.arc" << "stream*.arc"); }

bool MajiroEngine::inject()
{
  wchar_t process_name_[MAX_PATH]; // cached
  DWORD module_base_, module_limit_;
  if (!NtInspect::getCurrentProcessName(process_name_, MAX_PATH)) // Initialize process name
    return false;
  if (!NtInspect::getModuleMemoryRange(process_name_, &module_base_, &module_limit_))
    return false;
  DWORD addr = MemDbg::findCallerAddress((DWORD)TextOutA, 0xec81, module_base_, module_limit_);
  // Note: ITH will mess up this value
  addr = 0x41af90;
  if (!addr)
    return false;
  //growl::debug(*(BYTE*)addr);
  majiro::paint = detours::replace<majiro::paint_func_t>(addr, majiro::mypaint);
  //growl::debug((DWORD)((DWORD)majiro::paint == addr));
  //growl::debug((DWORD)majiro::paint);
  return addr;
}

// EOF
