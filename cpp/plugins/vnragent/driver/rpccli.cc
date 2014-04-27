// rpccli.cc
// 2/1/2013 jichi

#include "driver/rpccli.h"
#include "driver/rpccli_p.h"
#include <QtCore/QHash>

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

  connect(s, SIGNAL(disconnected()), q, SIGNAL(aborted()));
  connect(s, SIGNAL(error()), q, SIGNAL(aborted()));
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

  switch (qHash(cmd)) {
  case H_PING: ;

  case H_UI_CLEAR:      q_->emit clearUiRequested(); break;
  case H_UI_ENABLE:     q_->emit enableUiRequested(true); break;
  case H_UI_DISABLE:    q_->emit enableUiRequested(false); break;
  case H_UI_TEXT:       q_->emit uiTranslationReceived(param); break;

  case H_ENG_CLEAR:     q_->emit clearEngineRequested(); break;
  case H_ENG_ENABLE:    q_->emit enableEngineRequested(true); break;
  case H_ENG_DISABLE:   q_->emit enableEngineRequested(false); break;
  case H_ENG_TEXT:      q_->emit engineTranslationReceived(param); break;

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

bool RpcClient::isActive() const
{ return d_->r->isActive(); }

// - Requests -

void RpcClient::requestUiTranslation(const QString &json)
{ d_->sendUiTexts(json); }

void RpcClient::requestEngineTranslation(const QString &json)
{ d_->sendEngineTexts(json); }

void RpcClient::showMessage(const QString &t) { d_->growlServer(t, D::GrowlMessage); }
void RpcClient::showWarning(const QString &t) { d_->growlServer(t, D::GrowlWarning); }
void RpcClient::showError(const QString &t) { d_->growlServer(t, D::GrowlError); }
void RpcClient::showNotification(const QString &t) { d_->growlServer(t, D::GrowlNotification); }

// EOF
