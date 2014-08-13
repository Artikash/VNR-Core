#pragma once

// engine/engine.h
// 8/23/2013 jichi

#include "config.h"

namespace Engine {

void init(HANDLE hModule);
extern bool processAttached_; // the dll is loaded

} // namespace Engine

// EOF
