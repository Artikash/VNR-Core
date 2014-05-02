// rpcclient.cc
// 2/1/2013 jichi
#include "config.h"
#include "driver/rpcclient.h"
#include "driver/rpcclient_p.h"
#include "qtsocketsvc/bufferedsocketclient.h"
#include "qtsocketsvc/socketpack.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QHash>
#include <QtCore/QTimer>

#define DEBUG "rpcclient"
#include "sakurakit/skdebug.h"

/** Private class */

RpcClientPrivate::RpcClientPrivate(Q *q)
  : Base(q), q_(q), client(new RpcSocketClient(this))
{
  reconnectTimer = new QTimer(q);
  reconnectTimer->setSingleShot(false); // until reconnect successfully
  reconnectTimer->setInterval(ReconnectInterval);
  connect(reconnectTimer, SIGNAL(timeout()), SLOT(reconnect()));

  client->setPort(VNRAGENT_METACALL_PORT);
  client->setAddress(VNRAGENT_METACALL_HOST);

  connect(client, SIGNAL(dataReceived(QByteArray)), SLOT(onDataReceived(QByteArray)));

  connect(client, SIGNAL(disconnected()), q, SLOT(disconnected()));
  connect(client, SIGNAL(disconnected()), q, SIGNAL(aborted()));
  connect(client, SIGNAL(socketError()), q, SIGNAL(aborted()));

  connect(client, SIGNAL(socketError()), SLOT(reconnect()), Qt::QueuedConnection);
  connect(client, SIGNAL(disconnected()), reconnectTimer, SLOT(start()));
  connect(client, SIGNAL(socketError()), reconnectTimer, SLOT(start()));

}

bool RpcClientPrivate::reconnect()
{
  if (reconnectTimer->isActive())
    reconnectTimer->stop();
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

void RpcClientPrivate::sendData(const QByteArray &data)
{
#ifdef VNRAGENT_BUFFER_SOCKET
  client->sendDataLater(data, BufferInterval, WaitInterval);
#else
  client->sendData(data, WaitInterval);
#endif // VNRAGENT_BUFER_SOCKET
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
    H_PING          = 487495,   // "ping"
    H_UI_ENABLE     = 79990437,     // "ui.enable"
    H_UI_DISABLE    = 184943013,    // "ui.disable"
    H_UI_CLEAR      = 206185698,    // "ui.clear"
    H_UI_TEXT       = 197504020,    // "ui.text"
    H_ENG_ENABLE    = 207122565,    // "engine.enable"
    H_ENG_DISABLE   = 46785189,     // "engine.disable"
    H_ENG_CLEAR     = 230943490,    // "engine.clear"
    H_ENG_TEXT      = 81604852      // "engine.text"
  };

  if (args.isEmpty())
    return;

  switch (qHash(args.first())) {
  case H_PING: ;

  case H_UI_CLEAR:      q_->emit clearUiRequested(); break;
  case H_UI_ENABLE:     q_->emit enableUiRequested(true); break;
  case H_UI_DISABLE:    q_->emit enableUiRequested(false); break;
  case H_UI_TEXT:
    if (args.size() == 2)
      q_->emit uiTranslationReceived(args.last());
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

void RpcClient::requestUiTranslation(const QString &json) { d_->sendUiTexts(json); }

void RpcClient::sendEngineText(const QString &text, qint64 hash, int role, bool needsTranslation)
{ d_->sendEngineText(text, hash, role, needsTranslation); }

void RpcClient::growlMessage(const QString &t) { d_->growlServer(t, D::GrowlMessage); }
void RpcClient::growlWarning(const QString &t) { d_->growlServer(t, D::GrowlWarning); }
void RpcClient::growlError(const QString &t) { d_->growlServer(t, D::GrowlError); }
void RpcClient::growlNotification(const QString &t) { d_->growlServer(t, D::GrowlNotification); }

bool RpcClient::waitForDataReceived(int interval)
{ return d_->client->isConnected() && d_->client->waitForDataReceived(interval); }

// EOF
