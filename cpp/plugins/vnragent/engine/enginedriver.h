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
  void textsReceived(QString json); // [{role:int,hash:long,text:unicode}]
public slots:
  void updateTranslation(const QString &json); // [{role:int,hash:long,text:unicode}]
  void clearTranslation();
  void setEnable(bool t);

  // Called by engine
public:
  bool isEnabled() const;
};

// EOF
