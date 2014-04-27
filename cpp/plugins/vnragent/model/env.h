#pragma once

// engine.h
// 4/20/2014 jichi

#include <QtCore/QString>

namespace Env {

// File system
bool glob(const QString &nameFilter);
bool glob(const QStringList &nameFilters);

// Thread and process

QString getNormalizedProcessName();

bool getMemoryRange(const wchar_t *moduleName, unsigned long *startAddress, unsigned long *stopAddress);

} // namespace Env

// EOF
