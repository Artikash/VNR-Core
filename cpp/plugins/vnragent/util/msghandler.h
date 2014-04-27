#pragma once

// msghandler.h
// 4/27/2014 jichi

#include <QtCore/QtGlobal>

namespace Util {

void debugMsgHandler(QtMsgType type, const char *msg);
void installDebugMsgHandler();

} // namespace Util

// EOF
