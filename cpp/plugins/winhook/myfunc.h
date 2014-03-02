#pragma once
// myfunc.h
// 1/27/2013 jichi
#ifdef QT_CORE_LIB
# include <qt_windows.h>
#else
# include <windows.h>
#endif

namespace My {
void OverrideModules();
void OverrideModuleFunctions(HMODULE hModule);
} // namespace My

// EOF
