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
  static Self *instance(); // needed by Engine
  explicit EngineDriver(QObject *parent = nullptr);
  ~EngineDriver();

signals:
  void translationRequested(QString json); // {long hash:unicode text}
public slots:
  void updateTranslation(const QString &json); // {long hash:unicode translation}
  void clearTranslation();
  void abortTranslation();
  void setEnable(bool t);
public:
  void quit();

  // Called by engine
public:
  bool isEnabled() const;

  // Blocking
  QString translate(const QString &text, qint64 hash, bool block = true);
};

// EOF
