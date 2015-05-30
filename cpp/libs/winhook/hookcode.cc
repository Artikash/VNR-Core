// hookcode.cc
// 5/25/2015 jichi
#include "winhook/hookcode.h"
#include "winhook/hookutil_p.h"
#include "disasm/disasm.h"
#include "ccutil/ccmacro.h"
#include <windows.h>
#include <list>

namespace { // unnamed

namespace detail = winhook::detail; // import detail namespace

// Hook manager

class HookRecord
{
  typedef HookRecord Self;

  DWORD address_;           // the address being hooked
  BYTE *code_;              // code data to jump to, allocated with VirtualAlloc
  DWORD instructionSize_,   // actual size of the instruction at the address
        instructionOffset_; // index of the instruction to the code
  winhook::hook_function callback_; // hook callback

  //static void __thiscall callback(Self *self, DWORD esp) // callback from the function
  static void __fastcall callback(Self *ecx, void *edx, DWORD esp)
  {
    CC_UNUSED(edx);
    ecx->callback_((winhook::hook_stack *)esp);
  }

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
    BYTE *jmpCode = new BYTE[instructionSize_];
    jmpCode[0] = s1_jmp_;
    *(DWORD *)(jmpCode + 1) = (DWORD)code_ - (address_ + jmp_ins_size);
    if (instructionSize_ > jmp_ins_size)
      ::memset(jmpCode, s1_nop, instructionSize_ - jmp_ins_size); // patch nop

    bool ret = detail::protected_memcpy((LPVOID)address_, jmpCode, instructionSize_);
    delete jmpCode;
    return ret;
  }

  bool unhook() const // assume is valid
  {
    BYTE *instructionCode = new BYTE[instructionSize_];
    ::memcpy(instructionCode, code_ + instructionOffset_, instructionSize_);
    detail::move_code(instructionCode, instructionSize_, (DWORD)code_ + instructionOffset_, address_);
    bool ret = detail::protected_memcpy((LPVOID)address_, instructionCode, instructionSize_);
    delete instructionCode;
    return ret;
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
    prolog_ecx_offset = 3       // offset of s1_mov_ecx_0d
    , prolog_call_offset = 8    // offset of s1_call_0d
  };

// Ending of the hooked code, with the original instruction in the middle
//#define epilog_init
//  s1_jmp_0d // jmp 0,0,0,0

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
  detail::set_call_ins(code + prolog_call_offset, method);
  *(DWORD *)(code + prolog_ecx_offset + 1) = self;

  ::memcpy(code + prolog_size, (LPCVOID)address, instructionSize);
  detail::move_code(code + prolog_size, instructionSize, address, (DWORD)code + prolog_size);

  detail::set_jmp_ins(code + prolog_size + instructionSize, address + instructionSize);

  if (codeSize % 2)
    code[codeSize - 1] = s1_nop; // patch the last byte with int3 to be aligned;

  return code;
#undef prolog_init
//#undef epilog_init
}

class HookManager
{
  std::list<HookRecord *> hooks_; // not thread-safe

public:
  HookManager() {}
  ~HookManager() {} // HookRecord on heap are not deleted

  bool hook(DWORD address, const winhook::hook_function &callback)
  {
    size_t instructionSize = 0;
    while (instructionSize < jmp_ins_size) {
      size_t size = ::disasm((LPCVOID)address);
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
  return ::hookManager->hook(address, callback);
}

bool unhook(ulong address)
{ return ::hookManager && ::hookManager->unhook(address); }

WINHOOK_END_NAMESPACE

// EOF
