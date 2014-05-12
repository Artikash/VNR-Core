// pywinutil.cc
// 6/13/2013

#include "pywinutil/pywinutil.h"
#include "winshell/winshell.h"
#include <qt_windows.h>
#include <QtCore/QDir>
#include <QtCore>

QString WinUtil::resolveLink(const QString &input)
{
  wchar_t buf[MAX_PATH];
  QString path = QDir::toNativeSeparators(input);
  return WinShell::resolveLink((LPCWSTR)path.utf16(), buf, MAX_PATH) ?
      QString::fromWCharArray(buf) : QString();
}

QString WinUtil::toLongPath(const QString &input)
{
  wchar_t buf[MAX_PATH];
  QString path = QDir::toNativeSeparators(input);
  size_t size = ::GetLongPathNameW((LPCWSTR)path.utf16(), buf, MAX_PATH);
  return size ? QString::fromWCharArray(buf, size) : QString();
}

QString WinUtil::toShortPath(const QString &input)
{
  wchar_t buf[MAX_PATH];
  QString path = QDir::toNativeSeparators(input);
  size_t size = ::GetShortPathNameW((LPCWSTR)path.utf16(), buf, MAX_PATH);
  return size ? QString::fromWCharArray(buf, size) : QString();
}

// EOF
