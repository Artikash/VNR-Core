// rpccli.cc
// 2/1/2013 jichi

#include "driver/rpccli.h"
#include "driver/rpccli_p.h"
#include <QtCore/QHash>

//#define DEBUG "rpccli"
#include "growl.h"

/** Private class */

RpcClientPrivate::RpcClientPrivate(Q *q)
  : Base(q), q_(q), r(new RpcPropagator(this))
{
  reconnectTimer = new QTimer(q);
  reconnectTimer->setSingleShot(false); // until reconnect successfully
  reconnectTimer->setInterval(ReconnectInterval);
  connect(reconnectTimer, SIGNAL(timeout()), SLOT(reconnect()));

  connect(r, SIGNAL(agentMessageReceived(QString,QString)), SLOT(onMessage(QString,QString)));
  //connect(r, SIGNAL(updateClientData(QString)), q, SIGNAL(dataReceived(QString)));
  //connect(r, SIGNAL(callClient(QString)), SLOT(onCall(QString)));

  MetaCallSocketObserver *s = r->socketObserver();
  //connect(s, SIGNAL(disconnected()), SLOT(reconnect()), Qt::QueuedConnection);
  connect(s, SIGNAL(error()), SLOT(reconnect()), Qt::QueuedConnection);

  connect(s, SIGNAL(disconnected()), reconnectTimer, SLOT(start()));
  connect(s, SIGNAL(error()), reconnectTimer, SLOT(start()));
}

bool RpcClientPrivate::reconnect()
{
  if (reconnectTimer->isActive())
    reconnectTimer->stop();
  if (r->isActive())
    return true;
  r->stop();
  if (start()) {
    r->waitForReady();
    pingServer();
    return true;
  } else
    return false;
}

void RpcClientPrivate::onMessage(const QString &cmd, const QString &param)
{
  enum { // pre-computed qHash values
    H_PING          = 487495,   // "ping"
    H_UI_ENABLE     = 79990437,     // "ui.enable"
    H_UI_DISABLE    = 184943013,    // "ui.disable"
    H_UI_CLEAR      = 206185698,    // "ui.clear"
    H_UI_TEXT       = 197504020     // "ui.text"
  };

  switch (qHash(cmd)) {
  case H_UI_CLEAR:      q_->emit clearUiRequested(); break;
  case H_UI_ENABLE:     q_->emit enableUiRequested(true); break;
  case H_UI_DISABLE:    q_->emit enableUiRequested(false); break;
  case H_UI_TEXT:       q_->emit uiTranslationReceived(param); break;
  case H_PING: ;
  default: growl::debug(QString("Unknown command: %s").arg(cmd));
  }
}

/** Public class */

// - Construction -

RpcClient::RpcClient(QObject *parent)
  : Base(parent), d_(new D(this))
{
  if (!d_->reconnect())
    growl::debug(QString().sprintf("Visual Novel Reader is not ready! Maybe the port %i is blocked?", D::Port));
}

RpcClient::~RpcClient() { delete d_; }

bool RpcClient::isActive() const
{ return d_->r->isActive(); }

// - API -
void RpcClient::requestUiTranslation(const QString &json)
{ d_->sendUiTexts(json); }

void RpcClient::showMessage(const QString &t) { d_->growlServer(t, D::GrowlMessage); }
void RpcClient::showWarning(const QString &t) { d_->growlServer(t, D::GrowlWarning); }
void RpcClient::showError(const QString &t) { d_->growlServer(t, D::GrowlError); }

// EOF
