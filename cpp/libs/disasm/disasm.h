#pragma once
// disasm.h
// 1/27/2013 jichi

// Include typedef of BYTE
//#include <windef.h>
//#include <windows.h>

//#ifdef QT_CORE_LIB
//# include <qt_windows.h>
//#else
//# include <windows.h>
//#endif

#ifndef DISASM_BEGIN_NAMESPACE
# define DISASM_BEGIN_NAMESPACE
#endif
#ifndef DISASM_END_NAMESPACE
# define DISASM_END_NAMESPACE
#endif

DISASM_BEGIN_NAMESPACE
/**
 *  Get the length of the instruction.
 *  Warning: The current implementation is stateful and hence not thread-safe.
 *  @param  address of the instruction to look at
 *  @return  length of the instruction at the address or 0 if failed
 */
size_t disasm(const void *address);
DISASM_END_NAMESPACE

// EOF
