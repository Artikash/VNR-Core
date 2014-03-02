// growl.cc
// 2/1/2013
#include "winhook/growl.h"
#ifdef QT_CORE_LIB
# include <qt_windows.h>
#else
# include <windows.h>
#endif // QT_CORE_LIB

void growl::show(const char *message)
{ ::MessageBoxA(nullptr, message, "VNR Message", MB_OK); }

void growl::show(const wchar_t *message)
{ ::MessageBoxW(nullptr, message, L"VNR Message", MB_OK); }

void growl::warn(const char *message)
{ ::MessageBoxA(nullptr, message, "VNR Warning", MB_OK); }

void growl::warn(const wchar_t *message)
{ ::MessageBoxW(nullptr, message, L"VNR Warning", MB_OK); }

void growl::error(const char *message)
{ ::MessageBoxA(nullptr, message, "VNR Error", MB_OK); }

void growl::error(const wchar_t *message)
{ ::MessageBoxW(nullptr, message, L"VNR Error", MB_OK); }

#ifdef QT_CORE_LIB

void growl::show(const QString &message)
{ show(message.toStdWString().c_str()); }

void growl::warn(const QString &message)
{ warn(message.toStdWString().c_str()); }

void growl::error(const QString &message)
{ error(message.toStdWString().c_str()); }

#endif // QT_CORE_LIB

// EOF
