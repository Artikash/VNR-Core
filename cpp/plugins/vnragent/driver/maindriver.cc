// maindriver.cc
// 2/1/2013 jichi

#include "driver/maindriver.h"
#include "driver/maindriver_p.h"
#include "driver/rpcclient.h"
#include "driver/settings.h"
#include "engine/enginedriver.h"
#include "hijack/hijackdriver.h"
#include "hijack/hijackdriver.h"
#include "ui/uidriver.h"
#include "windbg/unload.h"

/** Public class */

MainDriver::MainDriver(QObject *parent) : Base(parent), d_(new D(this)) {}
MainDriver::~MainDriver() { delete d_; }

void MainDriver::quit() {}

/** Private class */

MainDriverPrivate::MainDriverPrivate(QObject *parent)
  : Base(parent)
{
  settings = new Settings(this);

  rpc = new RpcClient(this);
  {
    connect(rpc, SIGNAL(enableUiRequested(bool)), settings, SLOT(setUiTranslationEnabled(bool)));
    connect(rpc, SIGNAL(disconnected()), SLOT(onDisconnected()));
    connect(rpc, SIGNAL(detachRequested()), SLOT(detach()));
  }

  hijack = new HijackDriver(this);

  ui = new UiDriver(this); // TODO: Selective create ui only if enabled at server side, i.e. only called by rpc
  {
    connect(ui, SIGNAL(translationRequested(QString)), rpc, SLOT(requestUiTranslation(QString)));

    connect(settings, SIGNAL(uiTranslationEnabledChanged(bool)), ui, SLOT(setEnable(bool)));

    connect(rpc, SIGNAL(clearUiRequested()), ui, SLOT(clearTranslation()));
    connect(rpc, SIGNAL(uiTranslationReceived(QString)), ui, SLOT(updateTranslation(QString)));
  }

  eng = new EngineDriver(this); // TODO: Selective create engine only if enabled at server side, i.e. only called by rpc {
  {
    // Use queued connection to quarantine the engine thrread
    connect(eng, SIGNAL(textReceived(QString,qint64,int,bool)), rpc, SLOT(sendEngineText(QString,qint64,int,bool)),
        Qt::QueuedConnection);
    connect(rpc, SIGNAL(clearEngineRequested()), eng, SLOT(clearTranslation()));
    connect(rpc, SIGNAL(enableEngineRequested(bool)), eng, SLOT(setEnable(bool)));
    connect(rpc, SIGNAL(engineTranslationReceived(QString,qint64,int)), eng, SLOT(updateTranslation(QString,qint64,int)),
        Qt::QueuedConnection);
  }
}

void MainDriverPrivate::onDisconnected()
{
  settings->setUiTranslationEnabled(false);
}

void MainDriverPrivate::detach()
{
  WinDbg::unloadCurrentModule();
}

// EOF
