#pragma once

// hookutil.h
// 5/29/2015 jichi
#include "winhook/hookdef.h"

WINHOOK_BEGIN_NAMESPACE

/**
 *  Overwrite data at the target with the source data in the code section.
 *  @param  dst  target address to modify
 *  @param  src  address of the source data to copy
 *  @param  size  size of the source data to copy
 *  @return   if success
 */
bool csmemcpy(void *dst, const void *src, size_t size);

WINHOOK_END_NAMESPACE

// EOF
