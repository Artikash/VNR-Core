// rpccli.cc
// 2/1/2013 jichi

#include "winhook/qt/rpccli.h"
#include "winhook/qt/rpccli_p.h"
#include "winhook/growl.h"
#include "metacall/metacallpropagator.h"
#include <QtCore/QHash>

/** Private class */

RpcClientPrivate::RpcClientPrivate(Q *q)
  : Base(q), q_(q), r(new RpcPropagator(this))
{
  connect(r, SIGNAL(updateClientData(QString)), q, SIGNAL(dataReceived(QString)));
  connect(r, SIGNAL(callClient(QString)), SLOT(onCall(QString)));

  reconnectTimer.setInterval(3000);
  reconnectTimer.setMethod(this, &Self::reconnect);
  //connect(r, SIGNAL(disconnected()), SLOT(reconnect()), Qt::QueuedConnection);
  //connect(r, SIGNAL(socketError()), SLOT(reconnect()), Qt::QueuedConnection);
}

RpcClientPrivate::~RpcClientPrivate()
{ reconnectTimer.stop(); }

bool RpcClientPrivate::reconnect()
{
  if (r->isActive())
    return true;
  r->stop();
  if (start()) {
    r->waitForReady();
    r->emit q_pingServer(PORT);
    return true;
  } else
    return false;
}

void RpcClientPrivate::onCall(const QString &cmd)
{
  enum { // pre-computed qhash
    H_ENABLE    = 113539365, // "enable"
    H_DISABLE   = 185170405, // "disable"
    H_CLEAR     = 6957954    // "clear"
  };

  switch (qHash(cmd)) {
  case H_CLEAR:     q_->emit clearRequested(); break;
  case H_ENABLE:    q_->emit enableRequested(); break;
  case H_DISABLE:   q_->emit disableRequested(); break;
  default: ;
  }
}

/** Public class */

// - Construction -

RpcClient::RpcClient(QObject *parent)
  : Base(parent), d_(new D(this))
{
  if (!d_->reconnect())
    growl::warn(QString().sprintf("Visual Novel Reader is not ready! Maybe the port %i is blocked?", D::PORT));

  d_->reconnectTimer.start();
}

RpcClient::~RpcClient() { delete d_; }

bool RpcClient::isActive() const
{ return d_->r->isActive(); }

// - API -
void RpcClient::sendData(const QString &json)
{ d_->r->emit q_updateServerData(json); }

void RpcClient::showMessage(const QString &t) { d_->r->emit q_growlServerMessage(t); }
void RpcClient::showWarning(const QString &t) { d_->r->emit q_growlServerWarning(t); }
void RpcClient::showError(const QString &t)   { d_->r->emit q_growlServerError(t); }

// EOF
