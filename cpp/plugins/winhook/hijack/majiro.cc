// majiro.cc
// 4/20/2014 jichi

#include "hijack/majiro.h"
#include "hijack/majiro_p.h"
#include "detoursutil/detoursutil.h"
#include "ntdll/ntdll.h"
#include "growl.h"

namespace Util {
DWORD FindCallAndEntryAbs(DWORD fun, DWORD size, DWORD pt, DWORD sig);
BOOL FillRange(LPCWSTR name, DWORD *lower, DWORD *upper);
void GetProcessName(wchar_t *name);

// jichi 4/19/2014: Return the integer that can mask the signature
DWORD SigMask(DWORD sig)
{
  __asm
  {
    xor ecx,ecx
    mov eax,sig
_mask:
    shr eax,8
    inc ecx
    test eax,eax
    jnz _mask
    sub ecx,4
    neg ecx
    or eax,-1
    shl ecx,3
    shr eax,cl
  }
}
} // namespace Util

void Util::GetProcessName(wchar_t *name)
{
  //assert(name);
  PLDR_DATA_TABLE_ENTRY it;
  __asm
  {
    mov eax,fs:[0x30]
    mov eax,[eax+0xc]
    mov eax,[eax+0xc]
    mov it,eax
  }
  wcscpy(name, it->BaseDllName.Buffer);
}

DWORD Util::FindCallAndEntryAbs(DWORD fun, DWORD size, DWORD pt, DWORD sig)
{
  //WCHAR str[0x40];
  enum { reverse_length = 0x800 };
  DWORD mask = SigMask(sig);
  for (DWORD i = 0x1000; i < size - 4; i++)
    if (*(WORD *)(pt + i) == 0x15ff) {
      DWORD t = *(DWORD *)(pt + i + 2);
      if (t >= pt && t <= pt + size - 4) {
        if (*(DWORD *)t == fun)
          //swprintf(str,L"CALL addr: 0x%.8X",pt + i);
          //OutputConsole(str);
          for (DWORD j = i ; j > i - reverse_length; j--)
            if ((*(DWORD *)(pt + j) & mask) == sig) // Fun entry 1.
              //swprintf(str,L"Entry: 0x%.8X",pt + j);
              //OutputConsole(str);
              return pt + j;

      } else
        i += 6;
    }
  //OutputConsole(L"Find call and entry failed.");
  return 0;
}

BOOL Util::FillRange(LPCWSTR name, DWORD *lower, DWORD *upper)
{
  PLDR_DATA_TABLE_ENTRY it;
  LIST_ENTRY *begin;
  __asm
  {
    mov eax,fs:[0x30]
    mov eax,[eax+0xC]
    mov eax,[eax+0xC]
    mov it,eax
    mov begin,eax
  }

  while (it->SizeOfImage) {
    if (_wcsicmp(it->BaseDllName.Buffer, name)==0) {
      *lower = (DWORD)it->DllBase;
      *upper = *lower;
      MEMORY_BASIC_INFORMATION info = {};
      DWORD l,size;
      size = 0;
      do {
        NtQueryVirtualMemory(NtCurrentProcess(), (LPVOID)(*upper), MemoryBasicInformation, &info, sizeof(info), &l);
        if (info.Protect&PAGE_NOACCESS) {
          it->SizeOfImage=size;
          break;
        }
        size += info.RegionSize;
        *upper += info.RegionSize;
      } while (size < it->SizeOfImage);
      return TRUE;
    }
    it = (PLDR_DATA_TABLE_ENTRY)it->InLoadOrderModuleList.Flink;
    if (it->InLoadOrderModuleList.Flink == begin)
      break;
  }
  return FALSE;
}


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
  Util::GetProcessName(process_name_); // Initialize process name
  if (!Util::FillRange(process_name_, &module_base_, &module_limit_))
    return false;
  DWORD addr = Util::FindCallAndEntryAbs((DWORD)TextOutA, module_limit_ - module_base_, module_base_, 0xec81);
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
