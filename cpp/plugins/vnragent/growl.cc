// growl.cc
// 2/1/2013
#include "growl.h"
#ifdef QT_CORE_LIB
# include <qt_windows.h>
#else
# include <windows.h>
#endif // QT_CORE_LIB

void growl::debug(const char *message)
{ ::MessageBoxA(nullptr, message, "VNR Debug", MB_OK); }

void growl::debug(const wchar_t *message)
{ ::MessageBoxW(nullptr, message, L"VNR Debug", MB_OK); }

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

// EOF
