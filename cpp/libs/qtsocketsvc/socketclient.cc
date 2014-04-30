// socketclient.cc
// 4/29/2014 jichi
#include "qtsocketsvc/socketdef.h"
#include "qtsocketsvc/socketclient.h"
#include "qtsocketsvc/socketio_p.h"
#include <QtCore/QEventLoop>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>

#define DEBUG "socketclient"
#include <sakurakit/skdebug.h>

//QTSS_BEGIN_NAMESPACE

/** Private class */

class SocketClientPrivate
{
  SK_DECLARE_PUBLIC(SocketClient)
public:
  QTcpSocket *socket;
  int port;
  QString address;
  quint32 currentDataSize; // current message body size read from socket

  explicit SocketClientPrivate(Q *q)
    : q_(q), socket(nullptr), port(0), address(SOCKET_SERVICE_HOST), currentDataSize(0) {}

  void createSocket()
  {
    socket = new QTcpSocket(q_);
    q_->connect(socket, SIGNAL(readyRead()), SLOT(readSocket()));

    q_->connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SIGNAL(socketError()));
    q_->connect(socket, SIGNAL(connected()), SIGNAL(connected()));
    q_->connect(socket, SIGNAL(disconnected()), SIGNAL(disconnected()));
  }

  bool writeSocket(const QByteArray &data);
  QByteArray readSocket();

  void dumpSocketInfo() const; // for debug only
};


bool SocketClientPrivate::writeSocket(const QByteArray &data)
{ return socket && SocketService::writeSocket(socket, data); }

QByteArray SocketClientPrivate::readSocket()
{
  if (Q_UNLIKELY(!socket))
    return QByteArray();
  return SocketService::readSocket(socket, currentDataSize);
}

void SocketClientPrivate::dumpSocketInfo() const
{
  if (socket)
    DOUT("socket"
         ": localAddress ="<< socket->localAddress() <<
         ", localPort ="   << socket->localPort() <<
         ", peerAddress =" << socket->peerAddress() <<
         ", peerPort ="    << socket->peerPort() <<
         ", state ="       << socket->state() <<
         ", error ="       << socket->errorString());
  else
    DOUT("socket = null");
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
  d_->socket->connectToHost(QHostAddress(d_->address), d_->port);
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
  return d_->socket && d_->socket->state() == QAbstractSocket::ConnectedState;
}

bool SocketClient::isReady() const
{
  return d_->socket && (
    d_->socket->state() == QAbstractSocket::ConnectedState ||
    d_->socket->state() == QAbstractSocket::UnconnectedState
  );
}

void SocketClient::waitForReady()
{
  if (d_->socket &&
    d_->socket->state() != QAbstractSocket::ConnectedState &&
    d_->socket->state() != QAbstractSocket::UnconnectedState) {

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

bool SocketClient::sendData(const QByteArray &data, int waitTime)
{
  bool ok = d_->writeSocket(data);
  if (ok && waitTime)
    ok = d_->socket->waitForBytesWritten(waitTime);
  return ok;
}

bool SocketClient::waitForConnected(int interval)
{ return d_->socket && d_->socket->waitForConnected(interval); }

bool SocketClient::waitForDisconnected(int interval)
{ return d_->socket && d_->socket->waitForDisconnected(interval); }

bool SocketClient::waitForBytesWritten(int interval)
{ return d_->socket && d_->socket->waitForBytesWritten(interval); }

bool SocketClient::waitForReadyRead(int interval)
{ return d_->socket && d_->socket->waitForReadyRead(interval); }

void SocketClient::readSocket()
{
  if (Q_LIKELY(d_->socket))
    while (d_->socket->bytesAvailable()) {
      QByteArray data = d_->readSocket();
      if (data.isEmpty())
        break;
      else
        emit dataReceived(data);
    }
}

void SocketClient::dumpSocketInfo() const { d_->dumpSocketInfo(); }

//QTSS_END_NAMESPACE

// EOF
