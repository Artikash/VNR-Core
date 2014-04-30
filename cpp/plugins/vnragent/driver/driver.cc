// driver.cc
// 2/1/2013 jichi

#include "driver/driver.h"
#include "driver/driver_p.h"
#include "driver/rpccli.h"
#include "engine/enginedriver.h"
#include "ui/uidriver.h"

/** Public class */

Driver::Driver() : d_(new D) {}
Driver::~Driver() { delete d_; }

void Driver::quit() {}

/** Private class */

DriverPrivate::DriverPrivate(QObject *parent)
  : Base(parent)
{
  rpc = new RpcClient(this);

  ui = new UiDriver(this); // TODO: Selective create ui only if enabled at server side, i.e. only called by rpc
  {
    connect(ui, SIGNAL(translationRequested(QString)), rpc, SLOT(requestUiTranslation(QString)));
    connect(rpc, SIGNAL(clearUiRequested()), ui, SLOT(clearTranslation()));
    connect(rpc, SIGNAL(enableUiRequested(bool)), ui, SLOT(setEnable(bool)));
    connect(rpc, SIGNAL(uiTranslationReceived(QString)), ui, SLOT(updateTranslation(QString)));
  }

  //eng = new EngineDriver(this); // TODO: Selective create engine only if enabled at server side, i.e. only called by rpc {
  //{
  //  // Use queued connection to quarantine the engine thrread
  //  connect(eng, SIGNAL(textReceived(QString,qint64,int)), rpc, SLOT(sendEngineText(QString,qint64,int)),
  //          Qt::QueuedConnection);
  //  connect(rpc, SIGNAL(clearEngineRequested()), eng, SLOT(clearTranslation()));
  //  connect(rpc, SIGNAL(enableEngineRequested(bool)), eng, SLOT(setEnable(bool)));
  //  connect(rpc, SIGNAL(engineTranslationReceived(QString,qint64,int)), eng, SLOT(updateTranslation(QString,qint64,int)),
  //          Qt::QueuedConnection);
  //}
}

// EOF
