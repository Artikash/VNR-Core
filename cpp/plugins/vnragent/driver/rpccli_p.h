#pragma once

// rpccli_p.h
// 2/1/2013 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QObject>
#include <QtCore/QStringList>

QT_FORWARD_DECLARE_CLASS(QTimer)
class SocketClient;
class RpcClient;
class RpcClientPrivate : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(RpcClientPrivate)
  SK_DECLARE_PUBLIC(RpcClient)
  SK_EXTEND_CLASS(RpcClientPrivate, QObject)

  enum { ReconnectInterval = 5000 }; // reconnect on failed
  enum { WaitInterval = 5000 }; // wait for data sent
public:
  explicit RpcClientPrivate(Q *q);

  SocketClient *client;
  QTimer *reconnectTimer;


private slots:
  bool reconnect();
  void onDataReceived(const QByteArray &data);

private:
  void onCall(const QStringList &args); // called from server

  bool callServer(const QStringList &args); // call server

  void callServer(const QString &arg0, const QString &arg1)
  { callServer(QStringList() << arg0 << arg1); }

  void callServer(const QString &arg0, const QString &arg1, const QString &arg2)
  { callServer(QStringList() << arg0 << arg1 << arg2); }

  void callServer(const QString &arg0, const QString &arg1, const QString &arg2, const QString &arg3)
  { callServer(QStringList() << arg0 << arg1 << arg2 << arg3); }

  // Server calls, must be consistent with rpcman.py

  template<typename Number>
  static QString marshalNumber(Number value)
  { return QString::number(value); }
  //{ return QString::number(value, 16); }
  //{ return "0x" + QString::number(value, 16); }

  static int unmarshalInt(const QString &s)
  { return s.toInt(); }
  //{ return s.toInt(nullptr, 16); }
  //{ return s.startsWith("0x") ?  s.mid(2).toInt(nullptr, 16) : s.toInt(); }

  static qint64 unmarshalLongLong(const QString &s)
  { return s.toLongLong(); }
  //{ return s.toLongLong(nullptr, 16); }
  //{ return s.startsWith("0x") ?  s.mid(2).toLongLong(nullptr, 16) : s.toLongLong(); }

public:
  void pingServer();

  enum GrowlType {
    GrowlMessage = 0,
    GrowlWarning,
    GrowlError,
    GrowlNotification
  };
  void growlServer(const QString &msg, GrowlType t = GrowlMessage)
  {
    switch (t) {
    case GrowlMessage: callServer("growl.msg", msg); break;
    case GrowlWarning: callServer("growl.warn", msg); break;
    case GrowlError: callServer("growl.error", msg); break;
    case GrowlNotification: callServer("growl.notify", msg); break;
    }
  }

  void sendUiTexts(const QString &json) { callServer("agent.ui.text", json); }

  void sendEngineText(const QString &text, qint64 hash, int role)
  { callServer("agent.engine.text", text, marshalNumber(hash), marshalNumber(role)); }
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
