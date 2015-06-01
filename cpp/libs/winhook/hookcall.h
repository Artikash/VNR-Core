#pragma once

// hookcall.h
// 5/31/2015 jichi

#include "winhook/hookdef.h"

WINHOOK_BEGIN_NAMESPACE

/**
 *  Replace the old function definition with the new one
 *  @param  oldop  address of the call/jump instruction's parameter
 *  @param  newop  the function call to replace
 *  @return  original function address being call
 */
ulong replace_call(ulong oldop, ulong newop);

/**
 *  Restore hooked function
 *  @param  address  address being modified
 *  @return  address of the replaced function or 0 if failed
 */
//ulong restore_call(ulong address);

WINHOOK_END_NAMESPACE

// EOF
