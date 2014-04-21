// ntinspect.cc
// 4/20/2014 jichi
#include "ntdll/ntdll.h"
#include "ntinspect/ntinspect.h"

NTINSPECT_BEGIN_NAMESPACE

BOOL getCurrentProcessName(LPWSTR buffer, int bufferSize)
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
  return 0 == wcscpy_s(buffer, bufferSize, it->BaseDllName.Buffer);
}

BOOL getModuleMemoryRange(LPCWSTR moduleName, DWORD *lowerBound, DWORD *upperBound)
{
  //assert(lower);
  //assert(upper);
  PLDR_DATA_TABLE_ENTRY it;
  LIST_ENTRY *begin;
  __asm
  {
    mov eax,fs:[0x30]
    mov eax,[eax+0xc]
    mov eax,[eax+0xc]
    mov it,eax
    mov begin,eax
  }

  while (it->SizeOfImage) {
    if (_wcsicmp(it->BaseDllName.Buffer, moduleName) == 0) {
      DWORD lower = (DWORD)it->DllBase;
      if (lowerBound)
        *lowerBound = lower

      if (upperBound) {
        DWORD upper = lower;
        MEMORY_BASIC_INFORMATION info = {};
        DWORD size = 0;
        do {
          DWORD len;
          NtQueryVirtualMemory(NtCurrentProcess(), (LPVOID)upper, MemoryBasicInformation, &info, sizeof(info), &len);
          if (info.Protect & PAGE_NOACCESS) {
            it->SizeOfImage = size;
            break;
          }
          size += info.RegionSize;
          upper += info.RegionSize;
        } while (size < it->SizeOfImage);

        *upperBound = upper;
      }
      return TRUE;
    }
    it = (PLDR_DATA_TABLE_ENTRY)it->InLoadOrderModuleList.Flink;
    if (it->InLoadOrderModuleList.Flink == begin)
      break;
  }
  return FALSE;
}

BOOL getCurrentMemoryRange(DWORD *lowerBound, DWORD *upperBound)
{
  WCHAR procName[MAX_PATH]; // cached
  return getCurrentProcessName(procName, MAX_PATH)
      && NtInspect::getModuleMemoryRange(procName, lowerBound, upperBound);
}

NTINSPECT_END_NAMESPACE

// EOF
