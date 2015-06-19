#pragma once

// hook.h
// 8/23/2013 jichi
// Branch: ITH/IHF_DLL.h, rev 66

#include "ith/common/const.h"
#include "ith/common/types.h"

//#ifdef IHF
//# define IHFAPI __declspec(dllexport) __stdcall
//#else
//# define IHFAPI __declspec(dllimport) __stdcall
//#endif // IHF
#define IHFAPI // 9/19/2014 jichi: dummy

//DWORD IHFAPI OutputConsole(LPCWSTR text);
void IHFAPI ConsoleOutput(LPCSTR text); // jichi 12/25/2013: Used to return length of sent text
//DWORD IHFAPI OutputDWORD(DWORD d);
//DWORD IHFAPI OutputRegister(DWORD *base);
DWORD IHFAPI NotifyHookInsert(DWORD addr);
DWORD IHFAPI NewHook(const HookParam &hp, LPCWSTR name, DWORD flag = HOOK_ENGINE);
DWORD IHFAPI RemoveHook(DWORD addr);
DWORD IHFAPI SwitchTrigger(DWORD on);
DWORD IHFAPI GetFunctionAddr(const char *name, DWORD *addr, DWORD *base, DWORD *size, LPWSTR *base_name);
//DWORD IHFAPI RegisterEngineModule(DWORD idEngine, DWORD dnHook);

// 10/14/2014 jichi: disable GDI hooks
void EnableGDIHooks();
void EnableGDIPlusHooks();
void DisableGDIHooks();
void DisableGDIPlusHooks();
bool GDIHooksEnabled();
bool GDIPlusHooksEnabled();

// EOF
