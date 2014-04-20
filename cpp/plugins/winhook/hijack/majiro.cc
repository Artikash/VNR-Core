// majiro.cc
// 4/20/2014 jichi

#include "hijack/majiro.h"
#include "hijack/majiro_p.h"

/** Helpers */

#include <detours.h>
namespace detour {
// http://research.microsoft.com/en-us/projects/detours/
// Version 3.0 license costs $10000 orz
//
// Version 2.1:
// http://social.msdn.microsoft.com/Forums/en-US/vcgeneral/thread/ef4a6bdd-6e9f-4f0a-9096-ca07ad65ddc2/
// http://stackoverflow.com/questions/3263688/using-detours-for-hooking-writing-text-in-notepad
//BOOL (WINAPI *OldTextOutA)(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, int cchString) = TextOutA;
PVOID replace(_In_ PVOID oldfunc, _In_ PVOID &newfunc)  // version 2.0
{
  PVOID ret = oldfunc;
  ::DetourRestoreAfterWith();
  ::DetourTransactionBegin();
  ::DetourUpdateThread(::GetCurrentThread());
  ::DetourAttach(&ret, newfunc);
  ::DetourTransactionCommit();
  return ret;
}
} // detour

namespace Util {
DWORD FindCallAndEntryAbs(DWORD fun, DWORD size, DWORD pt, DWORD sig);
BOOL FillRange(LPCWSTR name, DWORD *lower, DWORD *upper);
void GetProcessName(wchar_t *name);
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
    return Majiro();
  return nullptr;
}

/** Majiro
 *  See: http://bbs.sumisora.org/read.php?tid=10983263
 */

namespace majiro {

LPVOID paint;
void mypaint()
{
  if (!paint)
    return;
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
  if (!addr)
    return false;
  majiro::paint = detour::replace(addr, majiro::mypaint);
  return addr;
}

// EOF
