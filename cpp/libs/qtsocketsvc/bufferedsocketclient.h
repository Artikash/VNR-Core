#ifndef _SOCKETSVC_BUFFEREDSOCKETCLIENT_H
#define _SOCKETSVC_BUFFEREDSOCKETCLIENT_H

// bufferedsocketclient.h
// 4/29/2014 jichi
//
// This class must be consistent with socketsvc/socketcli.py
#include "qtsocketsvc/socketclient.h"

class BufferedSocketClientPrivate;
class BufferedSocketClient : public SocketClient
{
  Q_OBJECT
  Q_DISABLE_COPY(BufferedSocketClient)
  SK_EXTEND_CLASS(BufferedSocketClient, SocketClient)
  SK_DECLARE_PRIVATE(BufferedSocketClientPrivate)
public:
  explicit BufferedSocketClient(QObject *parent = nullptr);
  ~BufferedSocketClient();

  void sendDataLater(const QByteArray &data, int interval = 200, int waitTime = 0);

public slots:
  void flushSendBuffer();
};

#endif // _SOCKETSVC_BUFFEREDSOCKETCLIENT_H
