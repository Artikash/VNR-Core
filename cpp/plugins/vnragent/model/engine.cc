// majiro.cc
// 4/20/2014 jichi

#include "model/engine.h"
#include "model/manifest.h"
#include <QtCore/QDir>
#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>

// - Creation -

AbstractEngine *AbstractEngine::getEngine()
{
  if (MajiroEngine::match())
    return new MajiroEngine;
  return nullptr;
}

// - Utilities -

bool AbstractEngine::glob(const QString &nameFilter)
{ return glob(QStringList(nameFilter)); }

bool AbstractEngine::glob(const QStringList &nameFilters)
{
  QDir cwd = QCoreApplication::applicationDirPath();
  return !cwd.entryList(nameFilters).isEmpty();
}

// EOF
