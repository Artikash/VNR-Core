#pragma once

// enginemanager.h
// 4/26/2014 jichi

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

public:
  void translationRequested(const QString &json);
  void updateTranslation(const QString &json); // received from the server
  void clearTranslation();
};

// EOF
