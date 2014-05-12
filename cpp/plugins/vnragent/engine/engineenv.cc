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

// - File -

bool Engine::glob(const QString &nameFilter)
{ return glob(QStringList(nameFilter)); }

bool Engine::glob(const QStringList &nameFilters)
{
  QDir dir = QCoreApplication::applicationDirPath();
  return !dir.entryList(nameFilters).isEmpty();
}

bool Engine::glob(const QString &relPath, const QString &nameFilter)
{ return glob(relPath, QStringList(nameFilter)); }

bool Engine::glob(const QString &relPath, const QStringList &nameFilters)
{
  QDir dir = QCoreApplication::applicationDirPath() + "/" + relPath;
  return dir.exists() && !dir.entryList(nameFilters).isEmpty();
}

bool Engine::exists(const QString &relPath)
{
  QString path = QCoreApplication::applicationDirPath() + "/" + relPath;
  return QFileInfo(path).exists();
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

  DWORD start, stop;

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
