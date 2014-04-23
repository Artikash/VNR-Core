#pragma once
// mygdi.h
// 4/14/2013 jichi
#include <windows.h>

namespace My {

void OverrideGDIModules();
void OverrideGDIModuleFunctions(HMODULE hModule);

} // namespace My

// EOF
