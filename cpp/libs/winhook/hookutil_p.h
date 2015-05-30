#pragma once

// hookutil_p.h
// 5/29/2015 jichi
#include "winhook/hookdef.h"
#include "winasm/winasmdef.h"

enum { max_ins_size = 8 }; // maximum individual instruction size
enum { jmp_ins_size = 5 }; // total size of jmp ????

WINHOOK_BEGIN_NAMESPACE

namespace detail {

/**
 *  Overwrite data at the target with the source data in the code region.
 *  @param  dst  target address to modify
 *  @param  src  address of the source data to copy
 *  @param  size  size of the source data to copy
 *  @return   if success
 */
bool protected_memcpy(void *dst, const void *src, size_t size);

/**
 *  @param  data  code data
 *  @param  address  the absolute address to jump to
 */
inline void set_jmp_ins(byte *data, ulong address)
{
  data[0] = s1_jmp_;
  *(ulong *)(data + 1) = address - ((ulong)data + jmp_ins_size);
}

} // namespace detail

WINHOOK_END_NAMESPACE

// EOF
