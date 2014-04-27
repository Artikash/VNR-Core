#pragma once

// rpccli_p.h
// 2/1/2013 jichi

#include "config.h"
#include "services/reader/metacall.h"
#include "qtmetacall/metacallobserver.h"
#include <QtCore/QObject>
#include <QtCore/QTimer>

// The only purpose of this class is to make signals in the propagator public
// No signals are allowed in this class
class RpcPropagator : public ReaderMetaCallPropagator
{
  Q_OBJECT
  Q_DISABLE_COPY(RpcPropagator)
  SK_EXTEND_CLASS(RpcPropagator, ReaderMetaCallPropagator)

  friend class RpcClient;
  friend class RpcClientPrivate;
public:
  explicit RpcPropagator(QObject *parent = nullptr)
    : Base(parent)
  {
    setSocketObserver(new MetaCallSocketObserver(this));
  }
};

class RpcClient;
class RpcClientPrivate : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(RpcClientPrivate)
  SK_DECLARE_PUBLIC(RpcClient)
  SK_EXTEND_CLASS(RpcClientPrivate, QObject)

  enum { ReconnectInterval = 5000 }; // reconnect on failed
public:
  enum { Port = VNRAGENT_METACALL_PORT };

  explicit RpcClientPrivate(Q *q);

  RpcPropagator *r;
  QTimer *reconnectTimer;

  bool start() { return r->startClient(VNRAGENT_METACALL_HOST, Port); }

private slots:
  bool reconnect();
  void onMessage(const QString &cmd, const QString &param);

private:
  void callServer(const QString &cmd, const QString &param) { r->emit serverMessageRequested(cmd, param); }
  void callServer(const QString &cmd) { r->emit serverMessageRequested(cmd, QString()); }

  // Server calls, must be consistent with rpcman.py
public:
  void pingServer() { callServer("ping"); }

  enum GrowlType { GrowlMessage = 0, GrowlWarning, GrowlError };
  void growlServer(const QString &msg, GrowlType t = GrowlMessage)
  {
    switch (t) {
    case GrowlMessage: callServer("growl.msg", msg); break;
    case GrowlWarning: callServer("growl.warn", msg); break;
    case GrowlError: callServer("growl.error", msg); break;
    }
  }

  void sendUiTexts(const QString &json) { callServer("ui.text", json); }
};

// EOF

/*
class RpcRouter : public MetaCallRouter
{
public:
  int convertSendMethodId(int value) override
  {
    switch (value) {
    case 6: return 16; // pingServer
    case 8: return 10; // pingClient
    case 10: return 14; // updateServerString
    default: return value;
    }
  }

  int convertReceiveMethodId(int value) override
  {
    switch (value) {
    case 12: return 12; // updateClientString
    default: return value;
    }
  }
};

*/
