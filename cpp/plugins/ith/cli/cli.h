#pragma once

// ihfdll.h
// 8/23/2013 jichi
// Branch: ITH/IHF_DLL.h, rev 66

#include "ith/common/const.h"
#include "ith/common/types.h"

#ifdef IHF
# define IHFAPI __declspec(dllexport) __stdcall
#else
# define IHFAPI __declspec(dllimport) __stdcall
#endif // IHF

extern "C" {
//DWORD IHFAPI OutputConsole(LPCWSTR text);
void IHFAPI ConsoleOutput(LPCSTR text); // jichi 12/25/2013: Used to return length of sent text
//DWORD IHFAPI OutputDWORD(DWORD d);
//DWORD IHFAPI OutputRegister(DWORD *base);
DWORD IHFAPI NotifyHookInsert(DWORD addr);
DWORD IHFAPI NewHook(const HookParam &hp, LPCWSTR name, DWORD flag = HOOK_ENGINE);
DWORD IHFAPI RemoveHook(DWORD addr);
DWORD IHFAPI RegisterEngineModule(DWORD base, DWORD idEngine, DWORD dnHook);
DWORD IHFAPI SwitchTrigger(DWORD on);
DWORD IHFAPI GetFunctionAddr(const char *name, DWORD *addr, DWORD *base, DWORD *size, LPWSTR *base_name);
} // extern "C"

// jichi 7/17/2014: Keept only for backward compatiblity
// This function should be deleted after removing MultiByte functions
void IHFAPI InsertNonGuiHooks();

// jichi 7/17/2014: Inesrt functions in gdi32.dll and user32.dll
void IHFAPI InsertGdiHooks();

void IHFAPI InsertLstrHooks();
void IHFAPI InsertWcharHooks();

// EOF
