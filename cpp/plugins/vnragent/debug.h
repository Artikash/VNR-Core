#pragma once

// debug.h
// 2/1/2013 jichi

#include <qt_windows.h>

#ifdef QT_CORE_LIB
# include <QtCore/QString>
#elif defined(_MSC_VER)
# include <cstddef> // for wchar_t
#endif

void dprint(const char *message)
{ ::MessageBoxA(nullptr, message, "VNR Message", MB_OK); }

void dprint(const char *message)
{ ::MessageBoxW(nullptr, message, "VNR Message", MB_OK); }

#ifdef QT_CORE_LIB
inline void dprint(const QString &message) { dprint(message.toStdWString().c_str()); }

template <typename T> inline void dprint(T number) { dprint(QString::number(number)); }
#endif //QT_CORE_LIB

#define DPRINT(...) dprint(__VA_ARGS__)

// EOF
