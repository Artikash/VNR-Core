#pragma once

// enginehash.h
// 4/26/2014 jichi

#include "sakurakit/skhash.h"
#include <QtCore/QString>

namespace Engine {

// Cast quint64 to qint64

inline qint64 hashByteArray(const QByteArray &b)
{ return Sk::djb2_n(reinterpret_cast<const quint8 *>(b.constData()), b.size()); }

inline qint64 hashString(const QString &s)
{ return Sk::djb2_n(reinterpret_cast<const quint8 *>(s.utf16()), s.size() * 2); }

inline qint64 hashCharArray(const void *lp, size_t len)
{ return Sk::djb2_n(reinterpret_cast<const quint8 *>(lp), len); }

inline qint64 hashWCharArray(const wchar_t *lp, size_t len)
{ return Sk::djb2_n(reinterpret_cast<const quint8 *>(lp), 2 * len); }

inline qint64 hashTextKey(qint64 hash, int role) { return hash + role; }

} // namespace Engine

// EOF
