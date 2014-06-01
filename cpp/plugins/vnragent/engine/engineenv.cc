// engineenv.cc
// 4/26/2014 jichi

#include "engine/engineenv.h"
#include "ntinspect/ntinspect.h"
#include <QtCore/QDir>
#include <QtCore/QCoreApplication>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>
#include <qt_windows.h>
#include <unordered_map>
#include <utility>

namespace { // unnamed

bool globsDir(const QDir &dir, const QString &filter)
{ return !dir.entryList(QStringList(filter)).isEmpty(); }

bool existsPath(const QString &path)
{ return QFileInfo(path).exists(); }

} // unnamed namespace

// - File -

bool Engine::globs(const QString &nameFilter)
{
  QDir dir = QCoreApplication::applicationDirPath();
  return ::globsDir(dir, nameFilter);
}

bool Engine::globs(const QStringList &nameFilters)
{
  QDir dir = QCoreApplication::applicationDirPath();
  foreach (const QString &filter, nameFilters)
    if (!::globsDir(dir, filter))
      return false;
  return true;
}

bool Engine::globs(const QString &relPath, const QString &nameFilter)
{
  QDir dir = QCoreApplication::applicationDirPath() + "/" + relPath;
  return dir.exists() && ::globsDir(dir, nameFilter);
}

bool Engine::globs(const QString &relPath, const QStringList &nameFilters)
{
  QDir dir = QCoreApplication::applicationDirPath() + "/" + relPath;
  if (!dir.exists())
    return false;
  foreach (const QString &filter, nameFilters)
    if (!::globsDir(dir, filter))
      return false;
  return true;
}

bool Engine::exists(const QString &relPath)
{
  QString path = QCoreApplication::applicationDirPath() + "/" + relPath;
  return ::existsPath(path);
}

bool Engine::exists(const QStringList &relPaths)
{
  QString base = QCoreApplication::applicationDirPath();
  foreach (const QString &path, relPaths)
    if (!::existsPath(base + "/" + path))
      return false;
  return true;
}

// - Process and threads -

QString Engine::getNormalizedProcessName()
{
  static QString ret;
  if (ret.isEmpty()) {
    QString path = QCoreApplication::applicationFilePath();
    ret = QFileInfo(path).fileName().toLower();
  }
  return ret;
}

bool Engine::getMemoryRange(const wchar_t *moduleName, unsigned long *startAddress, unsigned long *stopAddress)
{
  static std::unordered_map<const wchar_t *, std::pair<DWORD, DWORD> > cache;
  auto p = cache.find(moduleName);
  if (p != cache.end()) {
    auto pair = p->second;
    if (startAddress)
      *startAddress = pair.first;
    if (stopAddress)
      *stopAddress = pair.first;
    return true;
  }

  DWORD start,
        stop;

  wchar_t processName[MAX_PATH]; // cached
  if (!moduleName) {
    if (!NtInspect::getCurrentProcessName(processName, MAX_PATH)) // Initialize process name
      return false;
    moduleName = processName;
  }
  if (!NtInspect::getModuleMemoryRange(moduleName, &start, &stop))
    return false;
  cache[moduleName] = std::make_pair(start, stop);
  if (startAddress)
    *startAddress = start;
  if (stopAddress)
    *stopAddress = stop;
  return true;
}

unsigned long Engine::getModuleFunction(const char *moduleName, const char *funcName)
{
  if (HMODULE h = ::GetModuleHandleA(moduleName)) {
    DWORD ret = (DWORD)::GetProcAddress(h, funcName);
    ::CloseHandle(h);
    return ret;
  }
  return 0;
}

// EOF
