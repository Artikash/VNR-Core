#pragma once

// rpccli.h
// 2/1/2013 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QObject>

class RpcClientPrivate;
class RpcClient : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(RpcClient)
  SK_EXTEND_CLASS(RpcClient, QObject)
  SK_DECLARE_PRIVATE(RpcClientPrivate)

public:
  static Self *instance(); // only used by growl
  explicit RpcClient(QObject *parent = nullptr);
  ~RpcClient();
  bool isActive() const;

  // - API -
signals:
  void aborted(); // the last communication was aborted

  // UI
  void clearUiRequested();
  void enableUiRequested(bool t);
  void uiTranslationReceived(QString json); // json: {hash:text}

  // Engine
  void clearEngineRequested();
  void enableEngineRequested(bool t);
  void engineTranslationReceived(QString text, qint64 hash, int role); // json: {hash:text}

public slots:
  void sendEngineText(const QString &text, qint64 hash, int role);
  void requestUiTranslation(const QString &json); // json: {hash:text}

  void growlMessage(const QString &message);
  void growlWarning(const QString &message);
  void growlError(const QString &message);
  void growlNotification(const QString &message);

public:
  void quit(); // called before qApp->quit()
};

// EOF
