// majiro.cc
// 4/20/2014 jichi

#include "engine/majiro.h"
#include "engine/majiro_p.h"
#include "detoursutil/detoursutil.h"
#include "memdbg/memsearch.h"
#include "ntinspect/ntinspect.h"
#include "growl.h"

/** Engine */

namespace { // unnamed

namespace EngineData {
bool enabled = false;
} // namespace EngineData

} // unnamed namespace

bool Engine::isEnabled() { return EngineData::enabled; }
void Engine::setEnabled(bool t) { EngineData::enabled = t; }

Engine *Engine::getEngine()
{
  if (Majiro::match())
    return new Majiro;
  return nullptr;
}

/** Majiro
 *  See: http://bbs.sumisora.org/read.php?tid=10983263
 */

namespace majiro {

// int __cdecl sub_41AF90(CHAR String, int, LPCSTR lpString, int, int);
typedef int (* paint_func_t)(char, int, LPCSTR, int, int);
paint_func_t paint;
int mypaint(char ch, int x, LPCSTR str, int y, int z)
{
  return 0;
  //growl::debug("majiro::paint");
  //if (!paint)
  //  return 0;
  return paint(ch, x, str, y, z);
}

} // majiro

bool Majiro::match()
{
  // return glob("./data*.arc") and glob("./stream*.arc")
  return true;
}

bool Majiro::inject()
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
