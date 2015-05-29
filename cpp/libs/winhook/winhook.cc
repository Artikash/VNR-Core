// winhook.cc
// 5/25/2015 jichi
#include "winhook/winhook.h"
#include "winasm/winasmdef.h"
#include "disasm/disasm.h"
#include <windows.h>
#include <vector>

//#define WINHOOK_THREADSAFE // lock hook/unhook functions

#ifdef WINHOOK_THREADSAFE
# include "winmutex/winmutex.h"
# define HOOK_MANAGER_LOCK win_mutex_lock<HookManager::mutex_type> hook_manager_lock(::hookManager->mutex)
#else
# define HOOK_MANAGER_LOCK (void)0
#endif // WINHOOK_THREADSAFE

namespace { // unnamed

// Assembled binaries

enum { max_ins_size = 8 }; // maximum individual instruction size

enum { jmp_ins_size = 5 };
/**
 *  @param  tpl  template code data
 *  @param  address  target address to jump to
 */
inline void set_jmp_address(BYTE *tpl, DWORD address)
{ *(DWORD *)(tpl + 1) = address - (DWORD)tpl - jmp_ins_size; }

//inline DWORD compute_jmp_operand(DWORD src, DWORD dst)
//{ return dst - src - jmp_ins_size; }

// Beginning of the hooked code
#define hook_prolog \
    s1_pushad       /* 0 */ \
  , s1_pushfd       /* 1 */ \
  , s1_push_esp     /* 2 */ \
  , s1_mov_ecx_0d   /* 3    ecx = $this */ \
  , s1_call_0d      /* 8    call @hook */ \
  , s1_popfd        /* 13 */ \
  , s1_popad        /* 14 */ \
  //, s1_jmp_0d       /* 15   jmp after the hooked address  */

// Ending of the hooked code, with the original instruction in the middle
#define hook_epilog \
  s1_jmp_0d // jmp 0,0,0,0

/**
 *  @param  code  template code data
 *  @param  method  class method
 *  @param  self  class pointer or null
 *  @param  argument  the second argument to push after esp, supposed to be hooked address
 */
inline void set_hook_prolog(BYTE *code, DWORD method, DWORD self = 0)
{
  enum {
    hook_tpl_ecx_offset = 3 + 1     // offset of s1_mov_ecx_0d
    , hook_tpl_call_offset = 8 + 1  // offset of s1_call_0d
  };

  //*(DWORD *)(tpl + hook_tpl_push_offset) = argument;
  *(DWORD *)(code + hook_tpl_call_offset) = method;
  *(DWORD *)(code + hook_tpl_ecx_offset) = self;
}

/**
 *  @param  code  template code data
 *  @param  address  absolute address to jump to
 */
inline void set_hook_epilog(BYTE *code, DWORD address)
{ set_jmp_address(code, address); }

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
  DWORD oldProtect;
  //DWORD pid = ::GetCurrentProcessId();
  //HANDLE hProc = ::OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE, false, pid);
  //if (!hProc)
  //  return false;
  //if (!::VirtualProtectEx(hProc, dst, size, PAGE_EXECUTE_READWRITE, &oldProtect))
  if (!::VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect))
    return false;
  ::memcpy(dst, src, size);
  DWORD newProtect;
  ::VirtualProtect(dst, size, oldProtect, &newProtect); // the error code is not checked for this function
  return true;
}

/**
 *  @param  address  instruction address
 *  @return  size of the instruction at the address
 */
inline size_t get_ins_size(DWORD address)
{ return disasm((LPCVOID)address); }

// Type-cast helper
//inline bool protected_memcpy(DWORD dst, DWORD src, DWORD size)
//{ return protected_memcpy((LPVOID)dst, (LPCVOID)src, size); }

// Hook manager

// TODO:
// 1. Add win_mutex<CRITICAL_SECTON> to hook/unhook functions, add define/undefine for whether enable it
// 2. finish hook function
// 3. finish unhook function

struct HookRecord
{
  typedef HookRecord Self;

  BYTE *code; // code data to jump to, allocated with VirtualAlloc
  DWORD address; // the address being hooked
  size_t instructionSize;  // actual size of the instruction at the address
  winhook::hook_function hookFunction;

  //static void __fastcall callback(void *ecx, void *edx, DWORD esp);
  static void __thiscall callback(Self *self, DWORD esp)
  { self->hookFunction((winhook::hook_stack *)esp); }

  ~HookRecord() { ::VirtualFree(code); }

  /**
   *  @param  address   address to hook
   *  @param  instructionSize  size of the instruction at address
   *  @param  fun   hook function
   */
  HookRecord(DWORD address, DWORD instructionSize, const winhook::hook_function &fun)
    : address(address)
    , instructionSize(instructionSize)
    , hookFunction(fun)
  {
    static const BYTE prolog[] = { hook_prolog };
    static const BYTE epilog[] = { hook_epilog };
    enum { prolog_size = sizeof(prolog) };  // size of the prolog code
    enum { epilog_size = sizeof(epilog) };    // size of the epilog code

    size_t codeSize = prolog_size + instructionSize + epilog_size;
    if (codeSize % 2)
      codeSize++; // round CodeSize to 2

    *(DWORD *)(tpl + hook_tpl_jmp_offset) = address - (DWORD)tpl - hook_tpl_size;

    code = ::VirtualAlloc(nullptr, codeSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    ::memcpy(code, prolog_size, hook_tpl_size);
    ::memcpy(code + prolog_size, (LPCVOID)address, instructionSize);

    set_hook_prolog(code, (DWORD)&Self::callback, (DWORD)this);
    set_hook_epilog(code + prolog_size + instructionSize, address + instructionSize);

    if (codeSize % 2)
      code[codeSize - 1] = s1_int3; // patch the last byte with int3 to be aligned;
  }

};

class HookManager
{
  std::vector<HookRecord *> hooks_;

public:
#ifdef WINHOOK_THREADSAFE
  typedef win_mutex<CRITICAL_SECTION> mutex_type;
  mutex_type mutex; // mutex to lock translations
#endif // WINHOOK_THREADSAFE

  HookManager() {}
  ~HookManager() {} // HookRecord on heap are not deleted

  bool hook(DWORD address, const winhook::hook_function &callback)
  {
    size_t instructionSize = get_ins_size(address);
    if (!instructionSize)
      return false;
    HookRecord *h = new HookRecord(address, instructionSize, callback);

    BYTE jumpCode[max_ins_size] = { s1_jmp_0d };
    int jumpCodeSize = max(instructionSize, jmp_ins_size);
    set_jmp_address(jumpCode, address);

    if (!protected_memcpy((LPVOID)address, jumpCode, jumpCodeSize)) {
      delete h;
      return false;
    }
    hooks_.push_back(h);
    return true;
  }

  bool unhook(DWORD address)
  {
    for (auto it = hooks_.begin(); it != hooks_.end(); ++it) {
      HookRecord *p = *it;
      if (p->address == address) {
        bool ret = protected_memcpy((LPVOID)address, p->code + hook_tpl_size, p->instructionSize);
        hooks_.erase(it);
        delete p; // this will crash if the hook code is being executed
        return ret;
      }
    }
    return false;
  }
};

// Global variable
HookManager *hookManager;

} // unnamed namespace

WINHOOK_BEGIN_NAMESPACE

bool hook(ulong address, const hook_function &callback)
{
  if (!::hookManager)
    ::hookManager = new HookManager;
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
