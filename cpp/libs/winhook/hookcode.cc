// hookcode.cc
// 5/25/2015 jichi
#include "winhook/hookcode.h"
#include "winasm/winasmdef.h"
#include "disasm/disasm.h"
#include <windows.h>
#include <list>

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
enum { jmp_ins_size = 5 }; // total size of jmp ????

/**
 *  @param  tpl  template code data
 *  @param  address  target address to jump to
 */
//inline void set_jmp_inst(BYTE *code, DWORD address)
//{
//  *code = s1_jmp_;
//  *(DWORD *)(code + 1) = address - ((DWORD)tpl + jmp_ins_size);
//}

//inline DWORD compute_jmp_operand(DWORD src, DWORD dst)
//{ return dst - src - jmp_ins_size; }

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

class HookRecord
{
  typedef HookRecord Self;

  DWORD address_;           // the address being hooked
  BYTE *code_;              // code data to jump to, allocated with VirtualAlloc
  DWORD instructionSize_,   // actual size of the instruction at the address
        instructionOffset_; // index of the instruction to the code
  winhook::hook_function callback_; // hook callback

  //static void __fastcall callback(void *ecx, void *edx, DWORD esp);
  static void __thiscall callback(Self *self, DWORD esp) // callback from the function
  { self->callback_((winhook::hook_stack *)esp); }

  /**
   *  @param  address  instruction to hook
   *  @param  method  callback method
   *  @param  self  callback ecx
   *  @param  instructionSize
   *  @param[out]  instructionOffset  index of the instruction from the beginning of the code
   *  @return  code data created using VirtualAlloc
   */
  static BYTE *create_code(DWORD address, DWORD method, DWORD self, DWORD instructionSize, DWORD *instructionOffset);

public:
  /**
   *  @param  address   address to hook
   *  @param  instructionSize  size of the instruction at address
   *  @param  fun   hook function
   */
  HookRecord(DWORD address, DWORD instructionSize, const winhook::hook_function &fun)
    : address_(address)
    , instructionSize_(instructionSize)
    , callback_(fun)
  { code_ = create_code(address, (DWORD)&Self::callback, (DWORD)this, instructionSize, &instructionOffset_); }

  ~HookRecord() { if (code_) ::VirtualFree(code_, 0, MEM_RELEASE); }

  bool isValid() const { return code_ && address_ && instructionSize_ && instructionOffset_; }

  DWORD address() const { return address_; }

  bool hook() const // assume is valid
  {
    //assert(valid());
    int jmpCodeSize = max(instructionSize_, jmp_ins_size);
    BYTE *jmpCode = new BYTE[jmpCodeSize];
    jmpCode[0] = s1_jmp_;
    *(DWORD *)(jmpCode + 1) = address_ + instructionOffset_ - ((DWORD)jmpCode + jmp_ins_size);
    ::memset(jmpCode, s1_int3, jmpCodeSize - jmp_ins_size);

    bool ret = protected_memcpy((LPVOID)address_, jmpCode, jmpCodeSize);
    delete jmpCode;
    return ret;
  }

  bool unhook() const // assume is valid
  {
    //assert(valid());
    return protected_memcpy((LPVOID)address_, code_ + instructionOffset_, instructionSize_);
  }
};

BYTE *HookRecord::create_code(DWORD address, DWORD method, DWORD self, DWORD instructionSize, DWORD *instructionOffset)
{
// Beginning of the hooked code
#define prolog_init \
    s1_pushad       /* 0 */ \
  , s1_pushfd       /* 1 */ \
  , s1_push_esp     /* 2 */ \
  , s1_mov_ecx_0d   /* 3    ecx = $this */ \
  , s1_call_0d      /* 8    call @hook */ \
  , s1_popfd        /* 13 */ \
  , s1_popad        /* 14 */
  enum {
    prolog_ecx_offset = 3 + 1     // offset of s1_mov_ecx_0d
    , prolog_call_offset = 8 + 1  // offset of s1_call_0d
  };

// Ending of the hooked code, with the original instruction in the middle
#define epilog_init \
  s1_jmp_0d // jmp 0,0,0,0

  static const BYTE prolog[] = { prolog_init };
  //static const BYTE epilog[] = { epilog_init };
  enum { prolog_size = sizeof(prolog) };  // size of the prolog code
  enum { epilog_size = jmp_ins_size };    // size of the epilog code
  *instructionOffset = prolog_size; // output

  size_t codeSize = prolog_size + instructionSize + epilog_size;
  if (codeSize % 2)
    codeSize++; // round code size to 2, patch int3

  BYTE *code = (BYTE *)::VirtualAlloc(nullptr, codeSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

  ::memcpy(code, prolog, prolog_size);
  *(DWORD *)(code + prolog_call_offset) = method;
  *(DWORD *)(code + prolog_ecx_offset) = self;

  ::memcpy(code + prolog_size, (LPCVOID)address, instructionSize);

  DWORD jmpOffset = prolog_size + instructionSize; // offset of jmp
  code[jmpOffset] = s1_jmp_;
  *(DWORD *)(code + jmpOffset + 1) = address - ((DWORD)code + jmpOffset + jmp_ins_size);

  if (codeSize % 2)
    code[codeSize - 1] = s1_int3; // patch the last byte with int3 to be aligned;

  return code;
#undef hook_prolog
#undef hook_epilog
}

class HookManager
{
  std::list<HookRecord *> hooks_; // not thread-safe

public:
#ifdef WINHOOK_THREADSAFE
  typedef win_mutex<CRITICAL_SECTION> mutex_type;
  mutex_type mutex; // mutex to lock translations
#endif // WINHOOK_THREADSAFE

  HookManager() {}
  ~HookManager() {} // HookRecord on heap are not deleted

  bool hook(DWORD address, const winhook::hook_function &callback)
  {
    size_t instructionSize = 0;
    while (instructionSize < jmp_ins_size) {
      size_t size = get_ins_size(address);
      if (!size) // failed to decode instruction
        return false;
      instructionSize += size;
    }
    HookRecord *h = new HookRecord(address, instructionSize, callback);
    if (!h->isValid() || !h->hook()) {
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
      if (p->address() == address) {
        bool ret = p->isValid() && p->unhook();
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
