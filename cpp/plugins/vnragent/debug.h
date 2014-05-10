#pragma once

// debug.h
// 2/1/2013 jichi

#include <qt_windows.h>

#ifdef QT_CORE_LIB
# include <QtCore/QString>
#elif defined(_MSC_VER)
# include <cstddef> // for wchar_t
#endif

inline void dmsg(const char *message)
{ ::MessageBoxA(nullptr, message, "VNR Message", MB_OK); }

inline void dmsg(char *message)
{ ::MessageBoxA(nullptr, message, "VNR Message", MB_OK); }

inline void dmsg(const wchar_t *message)
{ ::MessageBoxW(nullptr, message, L"VNR Message", MB_OK); }

inline void dmsg(wchar_t *message)
{ ::MessageBoxW(nullptr, message, L"VNR Message", MB_OK); }

#ifdef QT_CORE_LIB

inline void dmsg(const QString &message) { dmsg(message.toStdWString().c_str()); }
template <typename T> inline void dmsg(T number) { dmsg(QString::number(number)); }

#endif //QT_CORE_LIB

#define DMSG(...) dmsg(__VA_ARGS__)
//#define DLOG(...) dlog(__VA_ARGS__)

// EOF
