#pragma once

// ntinspect.h
// 4/20/2014 jichi

#include <windows.h>
#ifndef MEMDBG_NO_STL
# include <functional>
#endif // MEMDBG_NO_STL

#ifndef NTINSPECT_BEGIN_NAMESPACE
# define NTINSPECT_BEGIN_NAMESPACE  namespace NtInspect {
#endif
#ifndef NTINSPECT_END_NAMESPACE
# define NTINSPECT_END_NAMESPACE    } // NtInspect
#endif


NTINSPECT_BEGIN_NAMESPACE

///  Get current module name in fs:0x30
BOOL getProcessName(_Out_ LPWSTR buffer, _In_ int bufferSize);

/**
 *  Get the memory range of the module if succeed
 *  @param  moduleName
 *  @param[out[  lowerBound
 *  @param[out]  upperBound
 *  @return  if succeed
 */
BOOL getModuleMemoryRange(_In_ LPCWSTR moduleName, _Out_ DWORD *lowerBound, _Out_ DWORD *upperBound);

///  Get memory of the current process module
BOOL getMemoryRange(_Out_ DWORD *lowerBound, _Out_ DWORD *upperBound);

/**
 *  Return the absolute address of the function imported from the given module
 *  @param  functionName
 *  @param* hModule  find from any module when null
 *  @return  function address or 0
 */
DWORD getModuleExportFunction(HMODULE hModule, LPCSTR functionName);

///  Get the function address exported from any module
DWORD getExportFunction(LPCSTR functionName);

/**
 *  Get the import address in the specified module
 *  @param  hModule
 *  @param  exportFunctionAddress  absolute address of the function exported from other modules
 *  @return  function address or 0
 */
DWORD getModuleImportAddress(HMODULE hModule, LPVOID exportFunctionAddress);

///  Get the import address in the current module
DWORD getImportAddress(LPVOID exportFunctionAddress);

#ifndef MEMDBG_NO_STL
///  Iterate module information and return false if abort iteration.
typedef std::function<bool (HMODULE hModule, LPCWSTR moduleName, size_t moduleSize)> iter_module_fun_t;

/**
 *  Iterate all modules
 *  @param  fun  the first parameter is the address of the caller, and the second parameter is the address of the call itself
 *  @return  false if return early, and true if iterate all elements
 */
bool iterModule(iter_module_fun_t fun);
#endif // MEMDBG_NO_STL


NTINSPECT_END_NAMESPACE

// EOF
