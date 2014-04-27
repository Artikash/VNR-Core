#pragma once

// driver_p.h
// 2/1/2013 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QObject>

class RpcClient;
class EngineDriver;
class UiDriver;

class DriverPrivate : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(DriverPrivate)
  SK_EXTEND_CLASS(DriverPrivate, QObject)

  RpcClient *rpc;
  EngineDriver *eng;
  UiDriver *ui;

public:
  explicit DriverPrivate(QObject *parent=nullptr);
  void quit();
};

// EOF
