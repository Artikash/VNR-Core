// socketclient.cc
// 4/29/2014 jichi
#include "qtsocketsvc/socketclient.h"
#include <QtCore/QEventLoop>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>

#define DEBUG "socketclient"
#include <sakurakit/skdebug.h>

QTSS_BEGIN_NAMESPACE

/** Private class */

class SocketClientPrivate
{
  SK_DECLARE_PUBLIC(SocketClient)
public:
  QTcpSocket *socket;
  int port;
  QString address;
  int currentDataSize; // current message body size read from socket

  explicit SocketClientPrivate(Q *q)
    : q_(q), socket(nullptr), port(0), currentDataSize(0) {}

  void createSocket()
  {
    socket = new QTcpSocket(this);
    q_->connect(socket, SIGNAL(readyRead()), SLOT(readSocket()));
  }

  bool writeSocket(const QByteArray &data);
  QByteArray readSocket();
};


bool SocketClientPrivate::writeSocket(const QByteArray &data)
{
  // CHECKPOINT
  if (!socket)
    return false;
  return false;
}

QByteArray SocketClientPrivate::writeSocket()
{
  // CHECKPOINT
  if (!socket)
    return QByteArray();
  return QByteArray();
}

/** Public class */

// Constructions:

SocketClient::SocketClient(QObject *parent)
  : Base(parent), d_(new D(this))
{}

SocketClient::~SocketClient() { delete d_; }

int SocketClient::port() const { return d_->port; }
void SocketClient::setPort(int value) { d_->port = value; }

QString SocketClient::address() const { return d_->address; }
void SocketClient::setAddress(const QString &value) { d_->address = value; }

void SocketClient::start()
{
  if (!d_->socket)
    d_->createSocket();
  d_->socket.connectToHost(QHostAddress(d_->address), d_->port);
  DOUT("pass");
}

void SocketClient::stop()
{
  if (d_->socket && d_->socket->isOpen()) {
    d_->socket->close();
    DOUT("pass");
  }
}

bool SocketClient::isActive() const
{
  return d_->socket && d_->socket.state() == QAbstractSocket::ConnectedState;
}

bool SocketClient::isReady() const
{
  return d_->socket && (
    d_->socket.state() == QAbstractSocket::ConnectedState ||
    d_->socket.state() == QAbstractSocket::UnconnectedState
  );
}

void SocketClient::waitForReady()
{
  if (d_->socket &&
    d_->socket.state() != QAbstractSocket::ConnectedState &&
    d_->socket.state() != QAbstractSocket::UnconnectedState) {

    QEventLoop loop;
    connect(d_->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), &loop, SLOT(quit()));
    connect(d_->socket, SIGNAL(error(QAbstractSocket::SocketError)), &loop, SLOT(quit()));
    do loop.exec();
    while (!d_->socket ||
           d_->socket->state() == QAbstractSocket::HostLookupState ||
           d_->socket->state() == QAbstractSocket::ConnectingState);
  }
}

// I/O:

bool SocketClient::sendData(const QByteArray &data)
{ return d_->writeSocket(data); }

void SocketClient::readSocket()
{
  if (d_->socket) {
    QByteArray data = d_->readSocket();
    if (!data.isEmpty())
      emit dataReceived(data);
  }
}

QTSS_END_NAMESPACE

// EOF
