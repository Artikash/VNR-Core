// winhook.cc
// 5/25/2015 jichi
#include "winhook/winhook.h"
#include "winasm/winasmdef.h"
#include <unordered_map>
#include <windows.h>

//#define WINHOOK_THREADSAFE // lock hook/unhook functions

#ifdef WINHOOK_THREADSAFE
# include "winmutex/winmutex.h"
# define HOOK_MANAGER_LOCK win_mutex_lock<HookManager::mutex_type> hook_manager_lock(::hookManager->mutex)
#else
# define HOOK_MANAGER_LOCK (void)0
#endif // WINHOOK_THREADSAFE

namespace { // unnamed

// Assembled binaries

const BYTE hook_tmpl[] = {
  s1_int3   // 1
  , s1_int3 // 2
  , s1_int3 // 3
  , s1_int3 // 4
  , s1_int3 // 5
  , s1_int3 // 6
  , s1_int3 // 7
  , s1_int3 // 8

  , s1_pushad
  , s1_pushfd
  , s1_push_esp
  , s1_push_0d      // -22  push hooked address
  , s1_mov_ecx_0d   // -17  ecx = $this
  , s1_call_0d      // -12  call @hook
  , s1_popfd        // -7
  , s1_popad        // -6
  , s1_jmp_0d       // -5   jmp after the hooked address
};
enum { hook_tmpl_size = sizeof(hook_tmpl) };

/**
 *  @param  tmpl  template code data
 *  @param  address  the address to jump to
 *  @param  method  class method
 *  @param  self  class pointer or null
 *  @param  argument  the second argument to push after esp, supposed to be hooked address
 */
inline void set_hook_tmpl(BYTE *tmpl, DWORD address, DWORD method, DWORD self = 0, DWORD argument = 0)
{
  enum {
    hook_tmpl_push_offset = hook_tmpl_size -22    // offset of s1_push_0d
    , hook_tmpl_ecx_offset = hook_tmpl_size -17   // offset of s1_mov_ecx_0d
    , hook_tmpl_call_offset = hook_tmpl_size -12  // offset of s1_call_0d
    , hook_tmpl_jmp_offset = hook_tmpl_size -5    // offset of s1_jmp_0d
  };

  *(DWORD *)(tmpl + hook_tmpl_push_offset) = argument;
  *(DWORD *)(tmpl + hook_tmpl_call_offset) = method;
  *(DWORD *)(tmpl + hook_tmpl_ecx_offset) = self;
  *(DWORD *)(tmpl + hook_tmpl_jmp_offset) = address;
}

// Helper functions

/**
 *  Overwrite data at the target with the source data in the code region.
 *  @param  dst  target address to modify
 *  @param  src  address of the source data to copy
 *  @param  size  size of the source data to copy
 *  @return   if success
 */
bool protected_memcpy(void *dst, const void *src, size_t size)
{
  DWORD pid = ::GetCurrentProcessId();
  DWORD oldProtect;
  HANDLE hProc = ::OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE, FALSE, pid);
  if (!hProc)
    return false;
  if (!::VirtualProtectEx(hProc, dst, size, PAGE_EXECUTE_READWRITE, &oldProtect))
    return false;
  ::memcpy(dst, src, size);
  DWORD newProtect;
  ::VirtualProtectEx(hProc, dst, size, oldProtect, &newProtect); // the error code is not checked for this function
  return true;
}

// Hook manager

// TODO:
// 1. Add win_mutex<CRITICAL_SECTON> to hook/unhook functions, add define/undefine for whether enable it
// 2. finish hook function
// 3. finish unhook function
struct HookRecord
{
  BYTE *originalCode; // original code data being modified
  size_t originalCodeSize;  // size of the original code data
  BYTE hookCode[hook_tmpl_size]; // code data to jump to
};

class HookManager
{
  std::unordered_map<DWORD, HookRecord *> m_;

public:
#ifdef WINHOOK_THREADSAFE
  typedef win_mutex<CRITICAL_SECTION> mutex_type;
  mutex_type mutex; // mutex to lock translations
#endif // WINHOOK_THREADSAFE

  HookManager() {}
  ~HookManager() {} // HookRecord on heap are not deleted

  HookRecord *lookupHook(DWORD address) const
  {
    auto p = m_.find(address);
    return p == m_.end() ? nullptr : p->second;
  }

  bool unhook(DWORD address)
  {
    // Not implemented
    return false;
  }

  bool hook(DWORD address, winhook::hook_fun_t callback)
  {
    return true;
  }
};

HookManager *hookManager;
HookManager *createHookManager() { return new HookManager; }

} // unnamed namespace

WINHOOK_BEGIN_NAMESPACE

bool hook(ulong address, hook_fun_t callback)
{
  if (!::hookManager)
    ::hookManager = ::createHookManager();
  HOOK_MANAGER_LOCK;
  return ::hookManager->hook(address, callback);
}

bool unhook(ulong address)
{
  if (!::hookManager)
    return true;
  HOOK_MANAGER_LOCK;
  return ::hookManager->unhook(address);
}

WINHOOK_END_NAMESPACE

// EOF
