#pragma once

// enginemanager.h
// 4/26/2014 jichi
// Game engine text manager.

#include "sakurakit/skglobal.h"
#include <QtCore/QList>
#include <QtCore/QObject>

class EngineManagerPrivate;
class EngineManager : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(EngineManager)
  SK_EXTEND_CLASS(EngineManager, QObject)
  SK_DECLARE_PRIVATE(EngineManagerPrivate)

public:
  explicit EngineManager(QObject *parent = nullptr);
  ~EngineManager();

  // Interface to RPC
signals:
  void translationRequested(const QString &json);
public:
  void quit();
  void updateTranslation(const QString &json); // received from the server
  void clearTranslation();
  void abortTranslation();

  // Interface to driver
public:
  void updateText(const QString &text, qint64 hash);
  QString findTranslation(qint64 hash) const;
  QString waitForTranslation(qint64 hash) const;
};

// EOF
