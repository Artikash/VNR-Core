// rpcclient.cc
// 2/1/2013 jichi
#include "config.h"
#include "driver/rpcclient.h"
#include "driver/rpcclient_p.h"
#include "qtsocketsvc/socketpack.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QHash>
#include <QtCore/QTimer>

#ifdef VNRAGENT_ENABLE_TCP_SOCKET
# ifdef VNRAGENT_ENABLE_BUFFERED_SOCKET
#  include "qtsocketsvc/bufferedtcpsocketclient.h"
# else
#  include "qtsocketsvc/tcpsocketclient.h"
# endif // VNRAGENT_ENABLE_BUFFERED_SOCKET
#else
# ifdef VNRAGENT_ENABLE_BUFFERED_SOCKET
#  include "qtsocketsvc/bufferedlocalsocketclient.h"
# else
#  include "qtsocketsvc/localsocketclient.h"
# endif // VNRAGENT_ENABLE_BUFFERED_SOCKET
#endif // VNRAGENT_ENABLE_TCP_SOCKET

#define DEBUG "rpcclient"
#include "sakurakit/skdebug.h"

/** Private class */

RpcClientPrivate::RpcClientPrivate(Q *q)
  : Base(q), q_(q), client(new RpcSocketClient(this))
{
#ifdef VNR_ENABLE_TCP_SOCKET
  client->setPort(VNRAGENT_SOCKET_PORT);
  client->setAddress(VNRAGENT_SOCKET_HOST);
#else
  client->setServerName(VNRAGENT_SOCKET_PIPE);
#endif // VNRAGENT_ENABLE_TCP_SOCKET

  connect(client, SIGNAL(dataReceived(QByteArray)), SLOT(onDataReceived(QByteArray)));

  connect(client, SIGNAL(disconnected()), q, SIGNAL(disconnected()));
  connect(client, SIGNAL(disconnected()), q, SIGNAL(aborted()));
  connect(client, SIGNAL(socketError()), q, SIGNAL(aborted()));

#ifdef VNRAGENT_ENABLE_RECONNECT
  reconnectTimer = new QTimer(q);
  reconnectTimer->setSingleShot(false); // until reconnect successfully
  reconnectTimer->setInterval(ReconnectInterval);
  connect(reconnectTimer, SIGNAL(timeout()), SLOT(reconnect()));

  connect(client, SIGNAL(socketError()), SLOT(reconnect()), Qt::QueuedConnection);
  connect(client, SIGNAL(disconnected()), reconnectTimer, SLOT(start()));
  connect(client, SIGNAL(socketError()), reconnectTimer, SLOT(start()));
#endif // VNRAGENT_ENABLE_RECONNECT
}

bool RpcClientPrivate::reconnect()
{
#ifdef VNRAGENT_ENABLE_RECONNECT
  if (reconnectTimer->isActive())
    reconnectTimer->stop();
#endif // VNRAGENT_ENABLE_RECONNECT
  if (client->isConnected())
    return true;
  //client->stop();
  client->start();
  if (client->waitForConnected())
    pingServer();
  return true;
}

void RpcClientPrivate::pingServer()
{
  auto pid = QCoreApplication::applicationPid();
  callServer("agent.ping", marshalInteger(pid));
}

void RpcClientPrivate::callServer(const QStringList &args)
{
  if (client->isConnected()) {
    QByteArray data = SocketService::packStringList(args);
    sendData(data);
  }
}

void RpcClientPrivate::callServerLater(const QStringList &args)
{
  if (client->isConnected()) {
    QByteArray data = SocketService::packStringList(args);
    sendDataLater(data);
  }
}

void RpcClientPrivate::sendData(const QByteArray &data)
{
#ifdef VNRAGENT_ENABLE_BUFFERED_SOCKET
  client->sendDataNow(data, WaitInterval);
#else
  client->sendData(data, WaitInterval);
#endif // VNRAGENT_ENABLE_BUFFERED_SOCKET
}

void RpcClientPrivate::sendDataLater(const QByteArray &data)
{
#ifdef VNRAGENT_ENABLE_BUFFERED_SOCKET
  client->sendDataLater(data);
#else
  sendData(data);
#endif // VNRAGENT_ENABLE_BUFFERED_SOCKET
}

void RpcClientPrivate::onDataReceived(const QByteArray &data)
{
  QStringList l = SocketService::unpackStringList(data);
  if (!l.isEmpty())
    onCall(l);
}

void RpcClientPrivate::onCall(const QStringList &args)
{
  enum { // pre-computed qHash(QString) values
    H_PING          = 487495        // "ping"
    , H_DETACH      = 111978392     // "detach"
    , H_WIN_ENABLE   = 147657733    // "window.enable"
    , H_WIN_DISABLE  = 195013157    // "window.disable"
    , H_WIN_CLEAR    = 160087362    // "window.clear"
    , H_WIN_TEXT     = 110732692    // "window.text"
    , H_ENG_ENABLE  = 207122565     // "engine.enable"
    , H_ENG_DISABLE = 46785189      // "engine.disable"
    , H_ENG_CLEAR   = 230943490     // "engine.clear"
    , H_ENG_TEXT    = 81604852      // "engine.text"
  };

  if (args.isEmpty())
    return;

  switch (qHash(args.first())) {
  case H_PING:          break;
  case H_DETACH:        q_->emit detachRequested(); break;

  case H_WIN_CLEAR:     q_->emit clearWindowTranslationRequested(); break;
  case H_WIN_ENABLE:    q_->emit enableWindowTranslationRequested(true); break;
  case H_WIN_DISABLE:   q_->emit enableWindowTranslationRequested(false); break;
  case H_WIN_TEXT:
    if (args.size() == 2)
      q_->emit windowTranslationReceived(args.last());
    break;

  case H_ENG_CLEAR:     q_->emit clearEngineRequested(); break;
  case H_ENG_ENABLE:    q_->emit enableEngineRequested(true); break;
  case H_ENG_DISABLE:   q_->emit enableEngineRequested(false); break;
  case H_ENG_TEXT:
    if (args.size() == 4) {
      QString text = args[1];
      qint64 hash = unmarshalLongLong(args[2]);
      int role = unmarshalLongLong(args[3]);
      q_->emit engineTranslationReceived(text, hash, role);
    }
    break;

  default: ; //growl::debug(QString("Unknown command: %s").arg(cmd));
  }
}

/** Public class */

// - Construction -

static RpcClient *instance_;
RpcClient *RpcClient::instance() { return ::instance_; }

RpcClient::RpcClient(QObject *parent)
  : Base(parent), d_(new D(this))
{
  if (!d_->reconnect()) { // connect on startup
    //growl::debug(QString().sprintf("Visual Novel Reader is not ready! Maybe the port %i is blocked?", D::Port));
  }

  ::instance_ = this;
}

RpcClient::~RpcClient() { ::instance_ = nullptr; }

bool RpcClient::isActive() const { return d_->client->isConnected(); }

// - Requests -

void RpcClient::requestWindowTranslation(const QString &json) { d_->sendWindowTexts(json); }

void RpcClient::sendEngineText(const QString &text, qint64 hash, int role, bool needsTranslation)
{ d_->sendEngineText(text, hash, role, needsTranslation); }

void RpcClient::sendEngineTextLater(const QString &text, qint64 hash, int role, bool needsTranslation)
{ d_->sendEngineTextLater(text, hash, role, needsTranslation); }

void RpcClient::growlMessage(const QString &t) { d_->growlServer(t, D::GrowlMessage); }
void RpcClient::growlWarning(const QString &t) { d_->growlServer(t, D::GrowlWarning); }
void RpcClient::growlError(const QString &t) { d_->growlServer(t, D::GrowlError); }
void RpcClient::growlNotification(const QString &t) { d_->growlServer(t, D::GrowlNotification); }

bool RpcClient::waitForDataReceived(int interval)
{ return d_->client->isConnected() && d_->client->waitForDataReceived(interval); }

// EOF
