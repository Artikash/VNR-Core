#pragma once

// main.h
// 1/31/2013 jichi

#include "wintimer/wintimer.h"
#include <windows.h>

namespace Main {

enum { EventLoopInterval = 500 };

/**
 *  Invoked when attached to the process
 *  @param  hInstance  the main dll instance
 *  @return  if OK
 */
void initWithInstance(HINSTANCE hInstance);

/**
 *  Invoked when detached from the process
 */
void destroy();

} // namespace Main

// EOF
