#pragma once

// wingui.h
// 2/1/2013 jichi

#ifdef QT_CORE_LIB
# include <qt_windows.h>
#else
# include <windows.h>
#endif

#ifndef WINGUI_BEGIN_NAMESPACE
# define WINGUI_BEGIN_NAMESPACE namespace wingui {
#endif
#ifndef WINGUI_END_NAMESPACE
# define WINGUI_END_NAMESPACE   } // namespace wingui
#endif

WINGUI_BEGIN_NAMESPACE

DWORD GetWindowProcessId(DWORD);
HWND GetAnyWindowWithThreadId(DWORD threadId);
HWND GetAnyWindowWithProcessId(DWORD processId);

WINGUI_END_NAMESPACE

// EOF
