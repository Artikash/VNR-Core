#ifndef _MEMDBG_MEMSEARCH_H
#define _MEMDBG_MEMSEARCH_H

// memsearch.h
// 4/20/2014 jichi

#include "memdbg/memdbg.h"

MEMDBG_BEGIN_NAMESPACE

/// Estimated maximum size of the caller function
enum { MaxCallerSize = 0x800 };

/**
 *  Return the absolute address of the caller function
 *  The same as ITH FindCallAndEntryAbs().
 *
 *  @param  funcAddr  callee function address
 *  @param  funcInst  the machine code where the caller function starts
 *  @param  lowerBound  the lower memory address to search
 *  @param  upperBound  the upper memory address to search
 *  @param* callerSearchSize  the maximum size of caller
 *  @return  the caller absolute address if succeed or 0 if fail
 *
 *  Example funcInst:
 *  0x55: push ebp
 *  0x81,0xec: sub esp XXOO (0xec81)
 *  0x83,0xec: sub esp XXOO (0xec83)
 */

dword_t findCallerAddress(dword_t funcAddr, dword_t funcInst, dword_t lowerBound, dword_t upperBound, dword_t callerSearchSize = MaxCallerSize);

/**
 *  Return the address of the first mached pattern.
 *  The same as ITH SearchPattern(). KMP is used.
 *  Return 0 if failed. The return result is ambiguous if the pattern address is 0.
 *
 *  @param  startAddress  search start address
 *  @param  range  search range
 *  @param  pattern  array of bytes to match
 *  @param  patternSize  size of the pattern array
 *  @return
 */
dword_t searchPattern(dword_t startAddress, dword_t range, const void *pattern, dword_t patternSize);

/**
 * jichi 2/5/2014: The same as SearchPattern except it uses 0xff to match everything
 * According to @Andys, 0xff seldom appear in the source code: http://sakuradite.com/topic/124
 */
enum : byte_t { SP_ANY = 0xff };
//#define SP_ANY_2 SP_ANY,SP_ANY
//#define SP_ANY_3 SP_ANY,SP_ANY,SP_ANY
//#define SP_ANY_4 SP_ANY,SP_ANY,SP_ANY,SP_ANY
dword_t searchPatternEx(dword_t startAddress, dword_t range, const void *pattern, dword_t patternSize, byte_t wildcard = SP_ANY);

MEMDBG_END_NAMESPACE

#endif // _MEMDBG_MEMSEARCH_H
