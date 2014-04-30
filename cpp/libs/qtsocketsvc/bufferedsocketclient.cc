// bufferedsocketclient.cc
// 4/29/2014 jichi
#include "qtsocketsvc/bufferedsocketclient.h"
#include "qtsocketsvc/socketpack.h"
#include <QtCore/QTimer>

/** Private class */

class BufferedSocketClientPrivate
{
  SK_DECLARE_PUBLIC(BufferedSocketClient)
public:
  QByteArray sendBuffer;
  QTimer *sendTimer;
  int sendWaitTime;

  explicit BufferedSocketClientPrivate(Q *q)
    : q_(q), sendWaitTime(0)
  {
    sendTimer = new QTimer(q);
    sendTimer->setSingleShot(true);
    q_->connect(sendTimer, SIGNAL(timeout()), SLOT(flushSendBuffer()));
  }

  void flushSendBuffer();
};


void BufferedSocketClientPrivate::flushSendBuffer()
{
  if (sendTimer->isActive())
    sendTimer->stop();
  if (!sendBuffer.isEmpty()) {
    q_->sendData(sendBuffer, sendWaitTime, false);
    sendBuffer.clear();
  }
}


/** Public class */

// Constructions:

BufferedSocketClient::BufferedSocketClient(QObject *parent)
  : Base(parent), d_(new D(this))
{}

BufferedSocketClient::~BufferedSocketClient() { delete d_; }

void BufferedSocketClient::flushSendBuffer() { d_->flushSendBuffer(); }

void BufferedSocketClient::sendDataLater(const QByteArray &data, int interval, int waitTime)
{
  d_->sendBuffer.append(SocketService::packData(data));
  d_->sendTimer->start(interval);
  d_->sendWaitTime = waitTime;
}

// EOF
