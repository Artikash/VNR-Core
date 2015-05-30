#pragma once

// hookfun.h
// 5/29/2015 jichi

#include "winhook/hookdef.h"

WINHOOK_BEGIN_NAMESPACE

// Helper structs
//struct fun_stack
//{
//  //ulong address;    // the input hooked address
//  ulong eflags;     // pushaf
//  ulong edi,        // pushad
//        esi,
//        ebp,
//        esp,
//        ebx,
//        edx,
//        ecx,        // this
//        eax;        // 0x28
//  ulong retaddr;    // return address
//  ulong args[1];    // first argument at runtime
//};

// FIXME: move_code not invoked, and hence the first a few instruction of the function cannot be jmp/call
/**
 *  Replace the old function with the new one
 *  @param  oldfun  address to insert jump
 *  @param  newfun  the function call to replace
 *  @return  entry point to access the old function or 0 if failed
 */
ulong replace_fun(ulong oldfun, ulong newfun);

/**
 *  Restore hooked function
 *  @param  address  address being modified
 *  @return  address of the replaced function or 0 if failed
 */
ulong restore_fun(ulong address);

WINHOOK_END_NAMESPACE

// EOF
