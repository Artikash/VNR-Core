#ifndef PYWINUTIL_H
#define PYWINUTIL_H

// pywinutil.h
// 6/13/2013 jichi

#include <QtCore/QString>

namespace WinUtil {
QString resolveLink(const QString &lnkfile);
QString toShortPath(const QString &longpath);
QString toLongPath(const QString &shortpath);
} // namespace WinUtil

#endif // PYWINUTIL
