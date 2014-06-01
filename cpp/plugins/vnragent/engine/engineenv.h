#pragma once

// engineenv.h
// 4/20/2014 jichi

#include <QtCore/QString>

namespace Engine {

// File system
bool globs(const QString &nameFilter);
bool globs(const QStringList &nameFilters);

bool globs(const QString &relPath, const QString &nameFilter);
bool globs(const QString &relPath, const QStringList &nameFilters);

bool exists(const QString &relPath);
bool exists(const QStringList &relPaths);

// Thread and process

QString getNormalizedProcessName();

bool getMemoryRange(const wchar_t *moduleName, unsigned long *startAddress, unsigned long *stopAddress);

inline bool getCurrentMemoryRange(unsigned long *startAddress, unsigned long *stopAddress)
{ return getMemoryRange(nullptr, startAddress, stopAddress); }

// This function might be cached and hence not thread-safe
unsigned long getModuleFunction(const char *moduleName, const char *funcName);

} // namespace Engine

// EOF
