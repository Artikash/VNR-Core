#pragma once

// engine/engine.h
// 8/23/2013 jichi

#include "config.h"

namespace Engine {

void init(LPVOID lpThreadParameter);

// jichi 10/21/2014: Return whether found the engine
bool IdentifyEngine();

// jichi 10/21/2014: Return 0 if failed
DWORD InsertDynamicHook(LPVOID addr, DWORD frame, DWORD stack);

} // namespace Engine

// EOF
