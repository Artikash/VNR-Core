// env.cc
// 4/26/2014 jichi

#include "model/env.h"
#include "ntinspect/ntinspect.h"
#include <QtCore/QDir>
#include <QtCore/QCoreApplication>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>
#include <qt_windows.h>
#include <unordered_map>
#include <utility>

// - File -

bool Env::glob(const QString &nameFilter)
{ return glob(QStringList(nameFilter)); }

bool Env::glob(const QStringList &nameFilters)
{
  static QDir cwd;
  if (cwd.path().isEmpty())
    cwd = QCoreApplication::applicationDirPath();
  return !cwd.entryList(nameFilters).isEmpty();
}

// - Process and threads -

QString Env::getNormalizedProcessName()
{
  static QString ret;
  if (ret.isEmpty()) {
    QString path = QCoreApplication::applicationFilePath();
    ret = QFileInfo(path).fileName().toLower();
  }
  return ret;
}

bool Env::getMemoryRange(const wchar_t *moduleName, unsigned long *startAddress, unsigned long *stopAddress)
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
