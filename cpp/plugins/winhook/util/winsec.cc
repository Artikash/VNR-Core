// winsec.cc
// 1/27/2013 jichi
#include "winsec.h"
#ifdef __cplusplus
# include <Tlhelp32.h>
# include <boost/foreach.hpp>
# include <list>
#endif // __cplusplus
#include <cwchar> // for wcslen

#ifdef _MSC_VER
# pragma warning (disable:4996)   // C4996: use POSIX function (stricmp)
#endif // _MSC_VER

//#define DEBUG "winsec"
#include "sakurakit/skdebug.h"

WINSEC_BEGIN_NAMESPACE

// - Remote Injection -

BOOL InjectFunction1(LPCVOID addr, LPCVOID data, SIZE_T dataSize, DWORD pid, HANDLE hProcess, INT timeout)
{
  DOUT("enter: pid =" <<  pid);
  if (hProcess == INVALID_HANDLE_VALUE && pid) {
     hProcess = ::OpenProcess(PROCESS_INJECT_ACCESS, FALSE, pid);
     // TODO: Privilege elevation is not implemented. See: skwinsec.py.
     //if (!hProcess) {
     //   priv = SkProcessElevator('SeDebugPrivilege')
     //   if not priv.isEmpty():
     //     handle = win32api.OpenProcess(PROCESS_INJECT_ACCESS, 0, pid)
     //}
  }
  if (hProcess == INVALID_HANDLE_VALUE) {
    DOUT("exit: error: failed to get process handle");
    return FALSE;
  }

  BOOL ret = FALSE;
  if (LPVOID remoteData = ::VirtualAllocEx(hProcess, nullptr, dataSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE)) {
    if (::WriteProcessMemory(hProcess, remoteData, data, dataSize, nullptr))
      if (HANDLE hThread = ::CreateRemoteThread(
          hProcess,
          nullptr, 0,
          reinterpret_cast<LPTHREAD_START_ROUTINE>(addr),
          remoteData,
          0, nullptr)) {
        ::WaitForSingleObject(hThread, timeout);
        ::CloseHandle(hThread);
        ret = TRUE;
      }
    ::VirtualFreeEx(hProcess, remoteData, dataSize, MEM_RELEASE);
  }
  ::CloseHandle(hProcess);
  DOUT("exit: ret =" << ret);
  return ret;
}

BOOL InjectDllW(LPCWSTR dllPath, DWORD pid, HANDLE hProcess, INT timeout)
{
  DOUT("enter: pid =" <<  pid);
  LPCVOID LOADLIBRARYW = GetModuleFunctionAddressA("LoadLibraryW", "kernel32.dll");
  if (!LOADLIBRARYW) {
    DOUT("exit error: cannot find LoadLibraryW from kernel32");
    return FALSE;
  }
  LPCVOID data = dllPath;
  SIZE_T dataSize = ::wcslen(dllPath) * 2 + 2; // L'\0'
  BOOL ok = InjectFunction1(LOADLIBRARYW, data, dataSize, pid, hProcess, timeout);
  DOUT("exit: ret =" << ok);
  return ok;
}

// - Inline Hook -
// See: http://asdf.wkeya.com/code/apihook6.html
PVOID OverrideFunctionA(HMODULE stealFrom, LPCSTR oldFunctionModule, LPCSTR functionName, LPCVOID newFunction)
{
  if (!stealFrom)
    return nullptr;
  //HMODULE oldModule = GetModuleHandleA(oldFunctionModule);
  //if (!oldModule)
  //  return nullptr;
  //void *originalAddress = GetProcAddress(oldModule, functionName);
  LPVOID originalAddress = GetModuleFunctionAddressA(functionName, oldFunctionModule);
  if (!originalAddress)
    return nullptr;
  IMAGE_DOS_HEADER *dosHeader = reinterpret_cast<IMAGE_DOS_HEADER *>(stealFrom);
  char *base = reinterpret_cast<char *>(stealFrom);
  if (::IsBadReadPtr(dosHeader, sizeof(IMAGE_DOS_HEADER)) || dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
    return nullptr;
  IMAGE_NT_HEADERS *ntHeader =
      reinterpret_cast<IMAGE_NT_HEADERS* >(base + dosHeader->e_lfanew);
  if (::IsBadReadPtr(ntHeader, sizeof(IMAGE_NT_HEADERS)) || ntHeader->Signature != IMAGE_NT_SIGNATURE)
    return nullptr;
  if (!ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress)
    return nullptr;
  IMAGE_IMPORT_DESCRIPTOR *import =
      reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR *>(base + ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

  // scan memory
  // TODO: add a maximum loop counter here!
  while (import->Name) {
    char *name = base + import->Name;
    if (!::stricmp(name, oldFunctionModule))
      break;
    import++;
  }
  if (!import->Name)
    return nullptr;
  IMAGE_THUNK_DATA *thunk = reinterpret_cast<IMAGE_THUNK_DATA *>(base + import->FirstThunk);
  while (thunk->u1.Function) {
    if ((ULONG_PTR)thunk->u1.Function == (ULONG_PTR)originalAddress) {
      ULONG_PTR *addr = reinterpret_cast<ULONG_PTR *>(&thunk->u1.Function);

      // See: http://asdf.wkeya.com/code/apihook6.html
      // Inline hook mechanism:
      //
      // LPVOID InlineHook3( PUINT8 mem, DWORD dwLen, PUINT8 pfOld, PUINT8 pfNew )
      // {
      //   DWORD dwOldProtect;
      //   VirtualProtect( ( PUINT8 )( pfOld ), dwLen, PAGE_READWRITE, &dwOldProtect );
      //   // 関数のエントリーから指定したbyte数をメモリの前方にコピー
      //   // メモリの数byte後方からオリジナルへのジャンプを作成
      //   // 指定の関数アドレスから5byteをフックへのjmp命令に書き換え
      //   VirtualProtect( ( PUINT8 )( pfOld ), dwLen, dwOldProtect, &dwOldProtect );
      //   return ( PVOID )mem;
      // }

      MEMORY_BASIC_INFORMATION mbi;
      if (::VirtualQuery((LPVOID)(addr), &mbi, sizeof(mbi)) == sizeof(mbi)) {
        DWORD dwOldProtect;
        if (::VirtualProtect(mbi.BaseAddress, ((ULONG_PTR)addr + 8)-(ULONG_PTR)mbi.BaseAddress, PAGE_EXECUTE_READWRITE, &dwOldProtect)) {
          *addr = (ULONG_PTR)newFunction;
          ::VirtualProtect(mbi.BaseAddress, ((ULONG_PTR)addr + 8)-(ULONG_PTR)mbi.BaseAddress, dwOldProtect, &dwOldProtect);
          return originalAddress;
        }
      }

    }
    thunk++;
  }
  return nullptr;
}

// - Process Suspender -

#ifdef __cplusplus

class ThreadsSuspenderPrivate
{
public:
  std::list<HANDLE> threads;
};

ThreadsSuspender::ThreadsSuspender(bool autoSuspend)
  : d_(new D)
{ if (autoSuspend) suspend(); }

ThreadsSuspender::~ThreadsSuspender()
{
  resume();
  delete d_;
}

void ThreadsSuspender::suspend()
{
  HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  if (hSnap == INVALID_HANDLE_VALUE)
    return;
  THREADENTRY32 entry;
  entry.dwSize = sizeof(entry);
  DWORD pid = ::GetCurrentProcessId();
  DWORD tid = ::GetCurrentThreadId();
  if (::Thread32First(hSnap, &entry))
    do if (entry.dwSize >= 4 * sizeof(DWORD) && entry.th32OwnerProcessID == pid && entry.th32ThreadID != tid) {
      if (HANDLE hThread = ::OpenThread(THREAD_SUSPEND_RESUME, 0, entry.th32ThreadID)) {
        if (::SuspendThread(hThread) != DWORD(-1))
          d_->threads.push_back(hThread);
        else
          ::CloseHandle(hThread);
      }
      entry.dwSize = sizeof(entry);
    } while (::Thread32Next(hSnap, &entry));
  ::CloseHandle(hSnap);
}

void ThreadsSuspender::resume()
{
  if (!d_->threads.empty()) {
    BOOST_FOREACH (HANDLE hThread, d_->threads) {
      ::ResumeThread(hThread);
      ::CloseHandle(hThread);
    }
    d_->threads.clear();
  }
}

#endif // __cplusplus

WINSEC_END_NAMESPACE

// EOF

/*
enum { CREATE_THREAD_ACCESS = (PROCESS_CREATE_THREAD |
                              PROCESS_QUERY_INFORMATION |
                              PROCESS_VM_OPERATION |
                              PROCESS_VM_WRITE |
                              PROCESS_VM_READ  ) };


int InjectDll(HANDLE hProcess, HINSTANCE hInst) {
  HANDLE hThread;

  wchar_t dllFile[2*MAX_PATH];
  if (GetModuleFileNameW(hInst, dllFile, sizeof(dllFile)/2) > sizeof(dllFile)/2) return 0;

  void *loadLibraryW = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
  if (!loadLibraryW) return 0;

  wchar_t *name;
  if (!(name = wcsrchr(dllFile, '\\'))) return 0;
  name ++;
  wcscpy(name, DLL_NAME);
  if (GetFileAttributes(dllFile) == INVALID_FILE_ATTRIBUTES) return 0;

  size_t len = sizeof(wchar_t)*(1+wcslen(dllFile));
  void *remoteString = (LPVOID)VirtualAllocEx(hProcess,
                     NULL,
                     len,
                     MEM_RESERVE|MEM_COMMIT,
                     PAGE_READWRITE
                    );
  if (remoteString) {
    if (WriteProcessMemory(hProcess, (LPVOID)remoteString, dllFile, len, NULL)) {
      if (hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)loadLibraryW, (LPVOID)remoteString, 0,0)) {
        WaitForSingleObject(hThread, 3000);
        CloseHandle(hThread);
        VirtualFreeEx(hProcess, remoteString, len, MEM_FREE);
        // Make sure it's injected before resuming.
        return 1;
      }
    }
    VirtualFreeEx(hProcess, remoteString, len, MEM_FREE);
  }
  return 0;
}

int getPriv(const char * name) {
  HANDLE hToken;
  LUID seValue;
  TOKEN_PRIVILEGES tkp;

  if (!LookupPrivilegeValueA(NULL, name, &seValue) ||
    !OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
      return 0;
  }

  tkp.PrivilegeCount = 1;
  tkp.Privileges[0].Luid = seValue;
  tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  int res = AdjustTokenPrivileges(hToken, 0, &tkp, sizeof(tkp), NULL, NULL);

  CloseHandle(hToken);
  return res;
}

inline int getDebugPriv() {
  return getPriv("SeDebugPrivilege");
}

int InjectIntoProcess(int pid) {
  HANDLE hProcess = OpenProcess(CREATE_THREAD_ACCESS, 0, pid);
  if (hProcess == 0) {
    getDebugPriv();
    hProcess = OpenProcess(CREATE_THREAD_ACCESS, 0, pid);
    if (!hProcess) return 0;
  }

  int out = InjectDll(hProcess);

  CloseHandle(hProcess);
  return out;
}
*/
