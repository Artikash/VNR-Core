#pragma once

// engineutil.h
// 4/20/2014 jichi

#include "memdbg/memsearch.h"
#include <QtCore/QString>

enum : quint8 { XX = MemDbg::WidecardByte }; // 0x11
#define XX2 XX,XX       // WORD
#define XX4 XX2,XX2     // DWORD
#define XX8 XX4,XX4     // QWORD

namespace Engine {

// Detours
typedef void *address_type;
typedef const void *const_address_type;

///  Replace the instruction at the old_addr with jmp to new_addr. Return the address to the replaced code.
address_type replaceFunction(address_type old_addr, const_address_type new_addr);
//address_type restoreFunction(address_type restore_addr, const_address_type old_addr);

// Ignore type checking
template<typename Ret, typename Arg1, typename Arg2>
inline Ret replaceFunction(Arg1 arg1, Arg2 arg2)
{ return (Ret)replaceFunction((address_type)arg1, (const_address_type)arg2); }

// Not used
//template<typename Ret, typename Arg1, typename Arg2>
//inline Ret restoreFunction(Arg1 arg1, Arg2 arg2)
//{ return (Ret)restoreFunction((address_type)arg1, (const_address_type)arg2); }

// File system
bool globs(const QString &relpath);
bool exists(const QString &relPath);

QStringList glob(const QString &nameFilter);

// Thread and process

QString getNormalizedProcessName();

//bool getMemoryRange(const char *moduleName, unsigned long *startAddress, unsigned long *stopAddress);
bool getMemoryRange(const wchar_t *moduleName, unsigned long *startAddress, unsigned long *stopAddress);

inline bool getCurrentMemoryRange(unsigned long *startAddress, unsigned long *stopAddress)
{ return getMemoryRange(nullptr, startAddress, stopAddress); }

// This function might be cached and hence not thread-safe
unsigned long getModuleFunction(const char *moduleName, const char *funcName);

bool isAddressReadable(const ulong *p);
bool isAddressReadable(const char *p, size_t count = 1);
bool isAddressReadable(const wchar_t *p, size_t count = 1);
bool isAddressWritable(const ulong *p);
bool isAddressWritable(const char *p, size_t count = 1);
bool isAddressWritable(const wchar_t *p, size_t count = 1);

} // namespace Engine

// EOF
