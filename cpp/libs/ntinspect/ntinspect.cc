// ntinspect.cc
// 4/20/2014 jichi
#include "ntdll/ntdll.h"
#include "ntinspect/ntinspect.h"

//#ifdef _MSC_VER
//# pragma warning(disable:4018) // C4018: signed/unsigned mismatch
//#endif // _MSC_VER

namespace { // unnamed

// Replacement of wcscpy_s which is not available on Windows XP's msvcrt
// http://sakuradite.com/topic/247
errno_t wcscpy_safe(wchar_t *buffer, size_t bufferSize, const wchar_t *source)
{
  size_t len = min(bufferSize - 1, wcslen(source));
  buffer[len] = 0;
  if (len)
    memcpy(buffer, source, len * 2);
  return 0;
}
} // unnamed namespace

NTINSPECT_BEGIN_NAMESPACE

/** Memory range */

BOOL getProcessName(LPWSTR buffer, int bufferSize)
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
  // jichi 6/4/2014: _s functions are not supported on Windows XP's msvcrt.dll
  //return 0 == wcscpy_s(buffer, bufferSize, it->BaseDllName.Buffer);
  return 0 == wcscpy_safe(buffer, bufferSize, it->BaseDllName.Buffer);
}

// See: ITH FillRange
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
        *lowerBound = lower;

      if (upperBound) {
        DWORD upper = lower;
        MEMORY_BASIC_INFORMATION mbi = {};
        DWORD size = 0;
        do {
          DWORD len;
          // Nt function is needed instead of VirtualQuery, which only works for the current process
          ::NtQueryVirtualMemory(NtCurrentProcess(), (LPVOID)upper, MemoryBasicInformation, &mbi, sizeof(mbi), &len);
          if (mbi.Protect & PAGE_NOACCESS) {
            it->SizeOfImage = size;
            break;
          }
          size += mbi.RegionSize;
          upper += mbi.RegionSize;
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

BOOL getMemoryRange(DWORD *lowerBound, DWORD *upperBound)
{
  WCHAR procName[MAX_PATH]; // cached
  return getProcessName(procName, MAX_PATH)
      && getModuleMemoryRange(procName, lowerBound, upperBound);
}

/** Module header */

// See: ITH AddModule
static DWORD getModuleExportFunction(LPCSTR funcName, DWORD hModule)
{
  IMAGE_DOS_HEADER *DosHdr = (IMAGE_DOS_HEADER *)hModule;
  if (IMAGE_DOS_SIGNATURE == DosHdr->e_magic) {
    DWORD dwReadAddr = hModule + DosHdr->e_lfanew;
    IMAGE_NT_HEADERS *NtHdr = (IMAGE_NT_HEADERS *)dwReadAddr;
    if (IMAGE_NT_SIGNATURE == NtHdr->Signature) {
      DWORD dwExportAddr = NtHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
      if (dwExportAddr == 0)
        return 0;
      dwExportAddr += hModule;
      IMAGE_EXPORT_DIRECTORY *ExtDir = (IMAGE_EXPORT_DIRECTORY *)dwExportAddr;
      dwExportAddr = hModule + ExtDir->AddressOfNames;
      for (UINT uj = 0; uj < ExtDir->NumberOfNames; uj++) {
        DWORD dwFuncName = *(DWORD *)dwExportAddr;
        char *pcFuncName = (char *)(hModule + dwFuncName);
        if (::strcmp(funcName, pcFuncName) == 0) {
          char *pcFuncPtr = (char *)(hModule + (DWORD)ExtDir->AddressOfNameOrdinals+(uj * sizeof(WORD)));
          WORD word = *(WORD *)pcFuncPtr;
          pcFuncPtr = (char *)(hModule + (DWORD)ExtDir->AddressOfFunctions+(word * sizeof(DWORD)));
          return hModule + *(DWORD *)pcFuncPtr; // absolute address
        }
        dwExportAddr += sizeof(DWORD);
      }
    }
  }
  return 0;
}

// See: ITH AddAllModules
DWORD getExportFunction(LPCSTR funcName, LPCWSTR moduleName)
{
  // Iterate loaded modules
  PPEB ppeb;
  __asm {
    mov eax, fs:[0x30]
    mov ppeb, eax
  }
  const DWORD start = *(DWORD *)(&ppeb->Ldr->InLoadOrderModuleList);
  for (PLDR_DATA_TABLE_ENTRY it = (PLDR_DATA_TABLE_ENTRY)start;
      it->SizeOfImage && *(DWORD *it) != start;
      it = (PLDR_DATA_TABLE_ENTRY)it->InLoadOrderModuleList.Flink) {
    if (moduleName && ::wcscmp(moduleName, it->BaseDllName.Buffer)) // BaseDllName.Buffer == moduleName
      continue;
    if (DWORD addr = getModuleExportFunctionAddress(funcName, it->DllBase))
      return addr;
  }
  return 0;
}

// See: ITH FindImportEntry
DWORD getImportAddress(DWORD hModule, DWORD fun)
{
  IMAGE_DOS_HEADER *DosHdr;
  IMAGE_NT_HEADERS *NtHdr;
  DWORD IAT, end, pt, addr;
  DosHdr = (IMAGE_DOS_HEADER *)hModule;
  if (IMAGE_DOS_SIGNATURE == DosHdr->e_magic) {
    NtHdr = (IMAGE_NT_HEADERS *)(hModule + DosHdr->e_lfanew);
    if (IMAGE_NT_SIGNATURE == NtHdr->Signature) {
      IAT = NtHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress;
      end = NtHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size;
      IAT += hModule;
      end += IAT;
      for (pt = IAT; pt < end; pt += 4) {
        addr = *(DWORD *)pt;
        if (addr == fun)
          return pt;
      }
    }
  }
  return 0;
}

NTINSPECT_END_NAMESPACE

// EOF
