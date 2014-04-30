#pragma once

// enginedriver.h
// 4/26/2014 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QObject>
#include <QtCore/QString>

class EngineDriverPrivate;
class EngineDriver : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(EngineDriver)
  SK_EXTEND_CLASS(EngineDriver, QObject)
  SK_DECLARE_PRIVATE(EngineDriverPrivate)

public:
  //static Self *instance(); // needed by Engine
  explicit EngineDriver(QObject *parent = nullptr);
  ~EngineDriver();

signals:
  void textReceived(QString text, qint64 hash, int role);
public slots:
  void updateTranslation(const QString &text, qint64 hash, int role);
  void clearTranslation();
  void setEnable(bool t);

  // Called by engine
public:
  bool isEnabled() const;
};

// EOF
