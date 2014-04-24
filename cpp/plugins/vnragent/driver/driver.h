#pragma once

// driver.h
// 2/1/2013 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QObject>

class DriverPrivate;
// Root object for all qobject
class Driver : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(Driver)
  SK_EXTEND_CLASS(Driver, QObject)
  SK_DECLARE_PRIVATE(DriverPrivate)
public:
  Driver();
  ~Driver();

  void quit();
};

// EOF
