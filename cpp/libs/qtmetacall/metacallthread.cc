// metacallthread.cc
// 4/27/2014 jichi
#include "qtmetacall/metacallthread.h"
#include "qtmetacall/metacallthread_p.h"
#include "qtmetacall/metacallobserver.h"
#include "qtmetacall/metacallfilter_p.h"
#include <QtCore/QEventLoop>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

#define DEBUG "metacallthread"
#include "sakurakit/skdebug.h"

/** Private class */

MetaCallThreadPrivate::MetaCallThreadPrivate(Q *q)
  : Base(q), q_(q), filter(nullptr), socketObserver(nullptr), server(nullptr), socket(nullptr)
{}

void MetaCallThreadPrivate::dumpSocket() const
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

void MetaCallThreadPrivate::createFilter(QObject *watched)
{
  Q_ASSERT(watched);
  filter = new MetaCallSocketFilter(this);
  filter->setWatchedObject(watched);
  watched->installEventFilter(filter);
}

// FIXME: According to this structure, it can only accept one client at a time
// The client has to reconnect on error.
void MetaCallThreadPrivate::serverAcceptsConnection()
{
  DOUT("enter");
  if (socket) {
    if (socket->isOpen())
      socket->close();
    socket->deleteLater();
  }
  if (server)
    socket = server->nextPendingConnection();
  else
    socket = nullptr;
  if (filter)
    filter->setSocket(socket);
  //dumpSocket();
  if (socket) {
#ifdef DEBUG
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(dumpSocket()));
    connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(dumpSocket()));
#endif // DEBUG
    connectSocketObserver();
  }
  DOUT("leave: ok =" << bool(socket));
}

void MetaCallThreadPrivate::connectSocketObserver()
{
  if (socket && socketObserver) {
    connect(socket, SIGNAL(connected()), socketObserver, SIGNAL(connected()));
    connect(socket, SIGNAL(disconnected()), socketObserver, SIGNAL(disconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), socketObserver, SIGNAL(error()));
  }
}

/** Public class */

// - Construction -

MetaCallThread::MetaCallThread(QObject *parent)
  : Base(parent), d_(new D(this)) {}

MetaCallThread::~MetaCallThread()
{
  stop();
  delete d_;
}

bool MetaCallThread::isServer() const
{ return d_->server; }

bool MetaCallThread::isClient() const
{ return !d_->server && d_->socket; }

MetaCallRouter *MetaCallThread::router() const
{ return d_->filter ? d_->filter->router() : nullptr; }

void MetaCallThread::setRouter(MetaCallRouter *value)
{
  if (!d_->filter)
    d_->createFilter(this);
  d_->filter->setRouter(value);
}

MetaCallSocketObserver *MetaCallThread::socketObserver() const
{ return d_->socketObserver; }

void MetaCallThread::setSocketObserver(MetaCallSocketObserver *value)
{
  if (d_->socketObserver != value) {
    if (d_->socketObserver)
      disconnect(d_->socketObserver);
    d_->socketObserver = value;
    if (value && d_->socket)
      d_->connectSocketObserver();
  }
}

// - Service -

bool MetaCallThread::isActive() const
{
  return d_->server && d_->server->isListening() ||
         d_->socket && d_->socket->state() == QAbstractSocket::ConnectedState;
}

bool MetaCallThread::isReady() const
{
  return !d_->socket ||
         d_->socket->state() == QAbstractSocket::ConnectedState ||
         d_->socket->state() == QAbstractSocket::UnconnectedState;
}

void MetaCallThread::waitForReady() const
{
  if (d_->socket &&
      d_->socket->state() != QAbstractSocket::ConnectedState &&
      d_->socket->state() != QAbstractSocket::UnconnectedState) {
    QEventLoop loop;
    connect(d_->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), &loop, SLOT(quit()));
    connect(d_->socket, SIGNAL(error(QAbstractSocket::SocketError)), &loop, SLOT(quit()));
    do loop.exec();
    while (d_->socket->state() == QAbstractSocket::HostLookupState ||
           d_->socket->state() == QAbstractSocket::ConnectingState);
  }
}

bool MetaCallThread::startServer(const QString &address, int port)
{
  DOUT("enter: address =" << address << ", port =" << port);
  if (d_->server) {
    DOUT("leave: ret = false, already started");
    return false;
  }

  d_->server = new QTcpServer(this);
  bool ok = d_->server->listen(QHostAddress(address), port);
  if (ok) {
    DOUT("server.connect: address =" << d_->server->serverAddress() << ", port =" << d_->server->serverPort());
    connect(d_->server, SIGNAL(newConnection()), d_, SLOT(serverAcceptsConnection()));
  } else {
    DOUT("error =" << d_->server->errorString());
    stop();
  }
  if (ok && !d_->filter)
    d_->createFilter(this);
  DOUT("leave: ret =" << ok);
  return ok;
}

bool MetaCallThread::startClient(const QString &address, int port)
{
  DOUT("enter: address =" << address << ", port =" << port);
  if (d_->socket) {
    DOUT("leave: ret = false, already started");
    return false;
  }

  d_->socket = new QTcpSocket(this);
#ifdef DEBUG
  connect(d_->socket, SIGNAL(error(QAbstractSocket::SocketError)), d_, SLOT(dumpSocket()));
  connect(d_->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), d_, SLOT(dumpSocket()));
#endif // DEBUG
  d_->socket->connectToHost(QHostAddress(address), port);

  if (!d_->filter)
    d_->createFilter(this);
  d_->filter->setSocket(d_->socket);

  d_->connectSocketObserver();

  DOUT("leave: ret = true");
  return true;
}

void MetaCallThread::stop()
{
  if (d_->filter)
    d_->filter->setSocket(nullptr);
  if (d_->socket) {
    if (d_->socket->isOpen())
      d_->socket->close();
    if (!d_->server)
      d_->socket->deleteLater();
    d_->socket = nullptr;
  }
  if (d_->server) {
    if (d_->server->isListening())
      d_->server->close();
    d_->server->deleteLater();
    d_->server = nullptr;
  }
}

// EOF
