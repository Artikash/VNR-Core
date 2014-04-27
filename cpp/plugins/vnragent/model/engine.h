#pragma once

// engine.h
// 4/20/2014 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QtGlobal>

QT_FORWARD_DECLARE_CLASS(QString)
QT_FORWARD_DECLARE_CLASS(QStringList)
class AbstractEngine
{
  SK_CLASS(AbstractEngine)
  SK_DISABLE_COPY(AbstractEngine)
public:
  static Self *getEngine();

  AbstractEngine() {}
  virtual ~AbstractEngine() {}
  virtual bool inject() = 0;

  //static bool isEnabled();
  //static void setEnabled(bool t);

protected:
  // FIXME: This function currently do not support subdirectory
  static bool glob(const QString &nameFilter);
  static bool glob(const QStringList &nameFilters);
};


// EOF
