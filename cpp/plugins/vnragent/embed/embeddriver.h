#pragma once

// embeddriver.h
// 4/26/2014 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QObject>
#include <QtCore/QString>

class EmbedDriverPrivate;
class EmbedDriver : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(EmbedDriver)
  SK_EXTEND_CLASS(EmbedDriver, QObject)
  SK_DECLARE_PRIVATE(EmbedDriverPrivate)

public:
  explicit EmbedDriver(QObject *parent = nullptr);
  ~EmbedDriver();

signals:
  void textReceived(QString text, qint64 hash, int role, bool needsTranslation);
  void textReceivedDelayed(QString text, qint64 hash, int role, bool needsTranslation);
  void engineNameChanged(QString name);
public slots:
  void updateTranslation(const QString &text, qint64 hash, int role);
  void clearTranslation();
  void setEnabled(bool t);

  // Called by engine
public:
  bool inject();
  bool isEnabled() const;
  void unload();
  QString engineName() const;
};

// EOF
