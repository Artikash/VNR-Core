#pragma once

// winsec.h
// 1/27/2013 jichi

#include "sakurakit/skglobal.h"

#ifdef QT_CORE_LIB
# include <qt_windows.h>
#else
# include <windows.h>
#endif

#ifndef WINSEC_BEGIN_NAMESPACE
# define WINSEC_BEGIN_NAMESPACE namespace winsec {
#endif
#ifndef WINSEC_END_NAMESPACE
# define WINSEC_END_NAMESPACE   } // namespace winsec
#endif

WINSEC_BEGIN_NAMESPACE

enum { PROCESS_INJECT_ACCESS = PROCESS_CREATE_THREAD|PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|PROCESS_VM_WRITE|PROCESS_VM_READ };
enum { INJECT_TIMEOUT = 3000 }; // wait at most 3 seconds for creating remote threads

///  Return the address of func in module.
inline FARPROC GetModuleFunctionAddressA(LPCSTR func, LPCSTR module = nullptr)
{ return ::GetProcAddress(::GetModuleHandleA(module), func); }

//inline FARPROC GetModuleFunctionAddressW(LPCSTR func, LPCWSTR module = nullptr)
//{ return ::GetProcAddress(::GetModuleHandleW(module), func); }

/**
 *  Inject function with 1 argument
 *  Either pid or the process handle should be specified
 *  @param  addr  LONG  function memory address
 *  @param  arg  LPVOID  arg1 data
 *  @param  argSize  int  arg1 data size
 *  @param  pid  process id
 *  @param  hProcess  process handle
 *  @param  timeout  msec
 *  @return  BOOL
 */
BOOL InjectFunction1(__in LPCVOID addr, __in LPCVOID arg, __in SIZE_T argSize,
    __in DWORD pid = 0, __in HANDLE hProcess = INVALID_HANDLE_VALUE,
    __in INT timeout = INJECT_TIMEOUT);

/**
 *  Either pid or the process handle should be specified
 *  @param  dllpath  ABSOLUTE path to dll
 *  @param  pid  process id
 *  @param  hProcess  process handle
 *  @param  timeout  msec
 *  @return  BOOL
 */
BOOL InjectDllW(__in LPCWSTR dllPath,
  __in DWORD pid = 0, __in HANDLE hProcess = INVALID_HANDLE_VALUE,
  __in INT timeout = INJECT_TIMEOUT);

/**
 * Replace the named function entry with the new one.
 * @param  stealFrom  instance of target module
 * @param  oldFunctionModule TODO
 * @param  functionName  name of the target function
 * @return  the orignal address if succeed, else nullptr
 *
 * See: http://www.codeproject.com/KB/DLL/DLL_Injection_tutorial.aspx
 */
PVOID OverrideFunctionA(__in HMODULE stealFrom, __in LPCSTR oldFunctionModule, __in LPCSTR functionName, __in LPCVOID newFunction);

#ifdef __cplusplus
class ThreadsSuspenderPrivate;
/**
 *  When created, automatically suspends all threads in the current process.
 *  When destroyed, resume suspended threads.
 */
class ThreadsSuspender
{
  SK_CLASS(ThreadsSuspender)
  SK_DISABLE_COPY(ThreadsSuspender)
  SK_DECLARE_PRIVATE(ThreadsSuspenderPrivate)

public:
  explicit ThreadsSuspender(bool autoSuspend = true);
  ~ThreadsSuspender();

  void resume(); ///<  Manually resume all threads
  void suspend(); ///<  Manually suspend all threads
};
#endif // __cplusplus

WINSEC_END_NAMESPACE

// EOF
