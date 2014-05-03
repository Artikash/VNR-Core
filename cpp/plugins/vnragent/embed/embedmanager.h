#pragma once

// embedmanager.h
// 4/26/2014 jichi
// Embedded game engine text manager.

#include "sakurakit/skglobal.h"
#include <QtCore/QList>
#include <QtCore/QObject>

class EmbedManagerPrivate;
class EmbedManager : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(EmbedManager)
  SK_EXTEND_CLASS(EmbedManager, QObject)
  SK_DECLARE_PRIVATE(EmbedManagerPrivate)

public:
  static Self *instance(); // needed by Engine

  explicit EmbedManager(QObject *parent = nullptr);
  ~EmbedManager();

  // Interface to RPC
signals:
  void textReceived(QString text, qint64 hash, int role, bool needsTranslation);
public:
  void updateTranslation(const QString &text, qint64 hash, int role);
  void clearTranslation();

  // Interface to engine
public:
  QString findTranslation(qint64 hash, int role) const;
  QString waitForTranslation(qint64 hash, int role) const;

  void addText(const QString &text, qint64 hash, int role, bool needsTranslation);
};

// EOF
