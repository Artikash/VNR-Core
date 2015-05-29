#pragma once

// hookcode.h
// 5/25/2015

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

// Function parameters
typedef std::function<void (hook_stack *)> hook_function;

/**
 *  Replace the instruction at address with a jump, invoke callback, and then return back.
 *  @param  address  address to insert jump
 *  @param  callback  the function call to replace
 *  @return  if succeed
 */
bool hook(ulong address, const hook_function &callback);

/**
 *  Restore hooked instruction
 *  @param  address  address where jump is inserted
 *  @return  if succeed
 */
bool unhook(ulong address);

WINHOOK_END_NAMESPACE

// EOF
