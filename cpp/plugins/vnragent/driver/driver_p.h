#pragma once

// driver_p.h
// 2/1/2013 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QObject>

class RpcClient;
class UiDriver;

class DriverPrivate : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(Driver)
  SK_EXTEND_CLASS(Driver, QObject)
  SK_DECLARE_PRIVATE(DriverPrivate)

  RpcClient *rpc;
  UiDriver *ui;

public:
  explicit DriverPrivate(QObject *parent=nullptr);
};

// EOF
