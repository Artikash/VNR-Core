#ifndef _MEMDBG_MEMSEARCH_H
#define _MEMDBG_MEMSEARCH_H

// memsearch.h
// 4/20/2014 jichi

#include "memdbg/memdbg.h"

MEMDBG_BEGIN_NAMESPACE

/// Estimated maximum size of the caller function
enum { MaximumFunctionSize = 0x800 };

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
dword_t findCallerAddress(dword_t funcAddr, dword_t funcInst, dword_t lowerBound, dword_t upperBound, dword_t callerSearchSize = MaximumFunctionSize);
dword_t findCallerAddressAfterInt3(dword_t funcAddr, dword_t lowerBound, dword_t upperBound, dword_t callerSearchSize = MaximumFunctionSize);
dword_t findLastCallerAddress(dword_t funcAddr, dword_t funcInst, dword_t lowerBound, dword_t upperBound, dword_t callerSearchSize = MaximumFunctionSize);
dword_t findLastCallerAddressAfterInt3(dword_t funcAddr, dword_t lowerBound, dword_t upperBound, dword_t callerSearchSize = MaximumFunctionSize);

/**
 *  Return the absolute address of the long jump (not short jump) instruction address.
 *  The same as ITH FindCallOrJmpAbs(false).
 *
 *  @param  funcAddr  callee function address
 *  @param  lowerBound  the lower memory address to search
 *  @param  upperBound  the upper memory address to search
 *  @return  the call instruction address if succeed or 0 if fail
 */
dword_t findJumpAddress(dword_t funcAddr, dword_t lowerBound, dword_t upperBound);

/**
 *  Return the absolute address of the far call (inter-module) instruction address.
 *  The same as ITH FindCallOrJmpAbs(true).
 *
 *  @param  funcAddr  callee function address
 *  @param  lowerBound  the lower memory address to search
 *  @param  upperBound  the upper memory address to search
 *  @return  the call instruction address if succeed or 0 if fail
 */
dword_t findFarCallAddress(dword_t funcAddr, dword_t lowerBound, dword_t upperBound);

///  Near call (intra-module)
dword_t findNearCallAddress(dword_t funcAddr, dword_t lowerBound, dword_t upperBound);

///  Default to far call
inline dword_t findCallAddress(dword_t funcAddr, dword_t lowerBound, dword_t upperBound)
{ return findFarCallAddress(funcAddr, lowerBound, upperBound); }

///  Push value >= 0xff
dword_t findPushDwordAddress(dword_t value, dword_t lowerBound, dword_t upperBound);

///  Push value <= 0xff
dword_t findPushByteAddress(byte_t value, dword_t lowerBound, dword_t upperBound);

///  Default to push DWORD
inline dword_t findPushAddress(dword_t value, dword_t lowerBound, dword_t upperBound)
{ return findPushDwordAddress(value, lowerBound, upperBound); }

/**
 *  Return the enclosing function address outside the given address.
 *  The same as ITH FindEntryAligned().
 *  "Aligned" here means the function must be after in3 (0xcc) or nop (0x90).
 *
 *  If the function does NOT exist, this function might raise without admin privilege.
 *  It is safer to wrap this function within SEH.
 *
 *  @param  addr  address within th function
 *  @param  searchSize  max backward search size
 *  @return  beginning address of the function
 */
dword_t findEnclosingAlignedFunction(dword_t addr, dword_t searchSize = MaximumFunctionSize);

/**
 *  Return the address of the first matched pattern.
 *  Return 0 if failed. The return result is ambiguous if the pattern address is 0.
 *
 *  @param  pattern  array of bytes to match
 *  @param  patternSize  size of the pattern array
 *  @param  lowerBound  search start address
 *  @param  upperBound  search stop address
 *  @return  absolute address
 */
dword_t findBytes(const void *pattern, dword_t patternSize, dword_t lowerBound, dword_t upperBound);

//inline dword_t findString(const char *pattern, dword_t patternSize, dword_t lowerBound, dword_t upperBound);
//{ return findBytes(pattern, patternSize, lowerBound, upperBound); }

/**
 * jichi 2/5/2014: The same as SearchPattern except it uses 0xff to match everything
 * According to @Andys, 0xff seldom appears in the source code: http://sakuradite.com/topic/124
 */
enum : byte_t { WidecardByte = 0xff };
//enum : WORD { WidecardWord = 0xffff };
dword_t findBytesWithWildcard(const void *pattern, dword_t patternSize, dword_t lowerBound, dword_t upperBound, byte_t wildcard = WidecardByte);

MEMDBG_END_NAMESPACE

#endif // _MEMDBG_MEMSEARCH_H
