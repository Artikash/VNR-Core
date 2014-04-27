#pragma once

// uidriver.h
// 2/1/2013 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QObject>

class UiDriverPrivate;
class UiDriver : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(UiDriver)
  SK_EXTEND_CLASS(UiDriver, QObject)
  SK_DECLARE_PRIVATE(UiDriverPrivate)

public:
  explicit UiDriver(QObject *parent = nullptr);

signals:
  void translationRequested(QString json); // {long hash : unicode text}
public slots:
  void updateTranslation(const QString &json); // {long hash : unicode translation}
  void clearTranslation();
  void setEnable(bool t);
public:
  bool isEnabled() const;
};

// EOF
