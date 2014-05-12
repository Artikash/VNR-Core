#pragma once

// engine_p.h
// 4/20/2014 jichi

#include "engine/engine.h"
#include <QtCore/QObject>

QT_FORWARD_DECLARE_CLASS(QTextCodec)
QT_FORWARD_DECLARE_CLASS(QTimer)

class EngineSettings;
class EngineSharedMemory;
class AbstractEnginePrivate : QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(AbstractEnginePrivate)
  SK_EXTEND_CLASS(AbstractEnginePrivate, QObject)
  SK_DECLARE_PUBLIC(AbstractEngine)

public:
  enum { ExchangeInterval = 10 };

  const char *name;
  Q::Encoding encoding;
  Q::RequiredAttributes attributes;

  QTextCodec *encoder,
             *decoder;

  EngineSettings *settings;

  EngineSharedMemory *exchangeMemory;
private:
  QTimer *exchangeTimer;

public:
  AbstractEnginePrivate(Q *q, const char *name, Q::Encoding encoding, Q::RequiredAttributes attributes);
  ~AbstractEnginePrivate();

  // Encoding

  QByteArray encode(const QString &text) const;
  QString decode(const QByteArray &data) const;

  void finalize();
private:
  void startExchange();
  void finalizeCodecs();
private slots:
  void exchange();
};

// EOF
