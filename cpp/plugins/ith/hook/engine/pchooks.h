#pragma once

// pchooks.h
// 8/1/2014 jichi

#include "config.h"

namespace PcHooks {

void hookGDIFunctions();
void hookGDIPlusFunctions();
void hookLstrFunctions();
void hookWcharFunctions();
void hookCharNextFunctions();

} // namespace PcHooks

// EOF
