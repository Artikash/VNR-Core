#pragma once

// hookcode.h
// 5/25/2015 jichi

#include "winhook/hookdef.h"
#include <functional>

WINHOOK_BEGIN_NAMESPACE

// Structs
struct hook_stack
{
  //ulong address;    // the input hooked address
  ulong eflags;     // pushaf
  ulong edi,        // pushad
        esi,
        ebp,
        esp,
        ebx,
        edx,
        ecx,        // this
        eax;        // 0x28
  ulong stack[1];   // beginning of the runtime stack
};

struct fun_stack
{
  //ulong address;    // the input hooked address
  ulong eflags;     // pushaf
  ulong edi,        // pushad
        esi,
        ebp,
        esp,
        ebx,
        edx,
        ecx,        // this
        eax;        // 0x28
  ulong retaddr;    // return address
  ulong args[1];    // first argument at runtime
};

// Return false if skip the next instruction.
typedef std::function<bool (hook_stack *)> hook_function;

/**
 *  Replace the instruction at address with a jump, invoke callback, and then return back.
 *  @param  address  address to insert jump
 *  @param  before  the function call to invoke just before the instruction at address
 *  @param  after   the function call to invoke just after the instruction at address
 *  @return  if succeed
 */
bool hook_both(ulong address, const hook_function &before, const hook_function &after);
bool hook_before(ulong address, const hook_function &before);
bool hook_after(ulong address, const hook_function &after);

/**
 *  Restore hooked instruction
 *  @param  address  address where jump is inserted
 *  @return  if succeed
 */
bool unhook(ulong address);

WINHOOK_END_NAMESPACE

// EOF
