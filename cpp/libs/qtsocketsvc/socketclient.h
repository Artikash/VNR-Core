#ifndef _QTSOCKETSVC_SOCKETCLIENT_H
#define _QTSOCKETSVC_SOCKETCLIENT_H

// socketclient.h
// 4/29/2014 jichi
//
// This class must be consistent with socketsvc/socketcli.py
#include "qtsocketsvc/qtsocketsvc.h"
#include "sakurakit/skglobal.h"
#include <QtCore/QObject>

QTSS_BEGIN_NAMESPACE

class SocketClientPrivate;
class SocketClient : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(SocketClient)
  SK_EXTEND_CLASS(SocketClient, QObject)
  SK_DECLARE_PRIVATE(SocketClient)
public:
  explicit SocketClient(QObject *parent = nullptr);
  ~SocketClient();

  // Properties:
  int port() const;
  void setPort(int value);

  QString address() const;
  void setAddress(const QString &value);

  bool isActive() const;
  bool isReady() const;

  // Actions:
public:
  void start();
  void stop();
  void waitForReady();

  bool sendData(const QByteArray &data);

signals:
  void connected();
  void disconnected();
  void socketError();
  void dataReceived(QByteArray data);

private slots:
  /**
   *  Invoked by QAbstractSocket::readyRead.
   *  It might be better to make this method virutal protected in the future.
   */
  void readSocket();
};

QTSS_END_NAMESPACE

#endif // _QTSOCKETSVC_SOCKETCLIENT_H
