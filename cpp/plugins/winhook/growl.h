#pragma once

// growl.h
// 2/1/2013 jichi

#ifdef QT_CORE_LIB
# include <QtCore/QString>
#elif defined(_MSC_VER)
# include <cstddef> // for wchar_t
#endif

namespace growl {

void show(const char *message);
void show(const wchar_t *message);

void warn(const char *message);
void warn(const wchar_t *message);

void error(const char *message);
void error(const wchar_t *message);

#ifdef QT_CORE_LIB
void show(const QString &message);
void warn(const QString &message);
void error(const QString &message);
#endif // QT_CORE_LIB

} // namespace growl

// EOF
