#pragma once

// detoursutil.h
// 4/20/2014 jichi

namespace detours {

typedef void *address_t; // LPVOID
typedef const void *const_address_t; // LPCVOID

// Replace old_addr with new_addr, and return the backup of old_addr if succeed or 0 if fails.
address_t replace(address_t old_addr, const_address_t new_addr);

// Ignore type checking
template<typename Ret, typename Old, typename New>
inline Ret replace(Old old_addr, New new_addr)
{ return (Ret)replace((address_t)old_addr, (const_address_t)new_addr); }

} // namespace detours

// EOF
