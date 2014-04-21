#ifndef _MEMDBG_MEMSEARCH_H
#define _MEMDBG_MEMSEARCH_H

// memsearch.h
// 4/20/2014 jichi

#include <windows.h>
// TO BE RENAMED
DWORD FindCallAndEntryAbs(DWORD funAddr, DWORD upperBound, DWORD lowerBound, DWORD entrySig);

#endif // _MEMDBG_MEMSEARCH_H
