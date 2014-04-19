#pragma once

// growl.h
// 2/1/2013 jichi

#ifdef QT_CORE_LIB
# include <QtCore/QString>
#elif defined(_MSC_VER)
# include <cstddef> // for wchar_t
#endif

namespace growl {

void debug(const char *message);
void debug(const wchar_t *message);

void show(const char *message);
void show(const wchar_t *message);

void warn(const char *message);
void warn(const wchar_t *message);

void error(const char *message);
void error(const wchar_t *message);

#ifdef QT_CORE_LIB
inline void debug(const QString &message) { debug(message.toStdWString().c_str()); }
inline void show(const QString &message) { show(message.toStdWString().c_str()); }
inline void warn(const QString &message) { warn(message.toStdWString().c_str()); }
inline void error(const QString &message) { error(message.toStdWString().c_str()); }

template <typename T> inline void debug(T number) { debug(QString::number(number)); }
template <typename T> inline void show(T number) { show(QString::number(number)); }
template <typename T> inline void warn(T number) { warn(QString::number(number)); }
template <typename T> inline void error(T number) { error(QString::number(number)); }

#endif // QT_CORE_LIB

} // namespace growl

// EOF
