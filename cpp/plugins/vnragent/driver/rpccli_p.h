#pragma once

// rpccli_p.h
// 2/1/2013 jichi

#include "config.h"
#include "sakurakit/skglobal.h"
#include <QtCore/QObject>
#include <QtCore/QStringList>

#ifdef VNRAGENT_BUFFER_SOCKET
class BufferedSocketClient;
typedef BufferedSocketClient RpcSocketClient;
#else
class SocketClient;
typedef SocketClient RpcSocketClient;
#endif // VNRAGENT_BUFFER_SOCKET

QT_FORWARD_DECLARE_CLASS(QTimer)

class RpcClient;
class RpcClientPrivate : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(RpcClientPrivate)
  SK_DECLARE_PUBLIC(RpcClient)
  SK_EXTEND_CLASS(RpcClientPrivate, QObject)

  enum { ReconnectInterval = 5000 }; // reconnect on failed

  enum { BufferInterval = 20 }; // wait for next buffered data
  enum { WaitInterval = 5000 }; // wait for data sent
public:
  explicit RpcClientPrivate(Q *q);

  RpcSocketClient *client;
  QTimer *reconnectTimer;


private slots:
  bool reconnect();
  void onDataReceived(const QByteArray &data);

private:
  void onCall(const QStringList &args); // called from server

  void callServer(const QStringList &args); // call server
  void sendData(const QByteArray &data);

  void callServer(const QString &arg0, const QString &arg1)
  { callServer(QStringList() << arg0 << arg1); }

  // Server calls, must be consistent with rpcman.py

  static bool unmarshalBool(const QString &s) { return s == "1"; }
  static int unmarshalInt(const QString &s) { return s.toInt(); }
  static qint64 unmarshalLongLong(const QString &s) { return s.toLongLong(); }

  static QString marshalBool(bool t) { return t ? '1' : '0'; }

  template<typename Number>
  static QString marshalInteger(Number value)
  { return QString::number(value); }

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

  void sendEngineText(const QString &text, qint64 hash, int role, bool needsTranslation)
  {
    callServer(QStringList()
        << "agent.engine.text"
        << text
        << marshalInteger(hash)
        << marshalInteger(role)
        << marshalBool(needsTranslation));
  }
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
