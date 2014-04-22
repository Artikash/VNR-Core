#pragma once

// funchook.h
// 1/27/2013

#include "winhook/winhook.h"

#ifdef _MSC_VER
# include <cstddef> // for wchar_t
#endif // _MSC_VER

WINHOOK_BEGIN_NAMESPACE

/**
 *  HookAfterFunction only works for stdcall functions.  Arguments to function are
 *  the output of the orignal function (Must be <= 32 bits) followed by the arguments
 *  to the original function.  Must return return value itself.
 *  Currently doesn't pass a parameter around, just takes param for future extension.
 *  hookFxn passed param and stack from the middle of a code point.
 */
int hookAfterFunction(void *addr, wchar_t *id, void *param, void *hookFxn);

// hookFxn passed param and stack from the middle of a code point.
int hookRawAddress(void *addr, wchar_t *id, void *param, void *hookFxn);

WINHOOK_END_NAMESPACE

// EOF
