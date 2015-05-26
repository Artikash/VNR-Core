#pragma once

// winhook.h
// 5/25/2015

#include "winhook/winhookdef.h"

WINHOOK_BEGIN_NAMESPACE

int replace(dword_t addr, dword_t jump);

WINHOOK_END_NAMESPACE

// EOF
