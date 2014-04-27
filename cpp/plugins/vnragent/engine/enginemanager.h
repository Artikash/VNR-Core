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
  static Self *instance(); // needed by Engine

  explicit EngineManager(QObject *parent = nullptr);
  ~EngineManager();

  // Interface to RPC
signals:
  void textsReceived(QString json); // [{role:int,hash:long,text:unicode}]
public:
  void updateTranslation(const QString &json); // [{role:int,hash:long,text:unicode}]
  void clearTranslation();

  // Interface to engine
public:
  QString findTranslation(qint64 hash, int role) const;
  QString waitForTranslation(qint64 hash, int role) const;
  void addText(const QString &text, qint64 hash, int role, void *context);

private slots:
  void submitTasks();
};

// EOF
