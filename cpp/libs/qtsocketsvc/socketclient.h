#ifndef _SOCKETSVC_SOCKETCLIENT_H
#define _SOCKETSVC_SOCKETCLIENT_H

// socketclient.h
// 4/29/2014 jichi
//
// This class must be consistent with socketsvc/socketcli.py
#include "sakurakit/skglobal.h"
#include <QtCore/QObject>

class SocketClientPrivate;
class SocketClient : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(SocketClient)
  SK_EXTEND_CLASS(SocketClient, QObject)
  SK_DECLARE_PRIVATE(SocketClientPrivate)
public:
  explicit SocketClient(QObject *parent = nullptr);
  ~SocketClient();

  // Properties:
  int port() const;
  void setPort(int value);

  QString address() const;
  void setAddress(const QString &value);

  bool isConnected() const;
  //bool isReady() const;

  // Actions:
public:
  void start();
  void stop();
  //void waitForReady();

  bool sendData(const QByteArray &data, int waitTime = 0, bool pack = true);

  // QAbstractSocket default wait time is 30 seconds
  bool waitForConnected(int interval = 30000);
  bool waitForDisconnected(int interval = 30000);
  bool waitForBytesWritten(int interval = 30000);
  bool waitForReadyRead(int interval = 30000);

  // Invoked just after the dataReceived is emit
  bool waitForDataReceived(int interval = 30000);

  void dumpSocketInfo() const; // for debug only

signals:
  void connected();
  void disconnected();
  void socketError();
  void dataReceived(QByteArray data);

private slots:
  /**
   *  Invoked by QAbstractSocket::readyRead.
   *  Leave it in the public class as it  might be better to use  virutal protected in the future.
   */
  void readSocket();
};

#endif // _SOCKETSVC_SOCKETCLIENT_H
