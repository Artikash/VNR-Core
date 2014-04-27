// driver.cc
// 2/1/2013 jichi

#include "driver/driver.h"
#include "driver/driver_p.h"
#include "driver/rpccli.h"
#include "engine/enginedriver.h"
#include "ui/uidriver.h"
#include "model/engine.h"

/** Public class */

Driver::Driver() : d_(new D) {}
Driver::~Driver() { delete d_; }

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

  eng = new EngineDriver(this); // TODO: Selective create engine only if enabled at server side, i.e. only called by rpc {
  {
    connect(eng, SIGNAL(translationRequested(QString)), rpc, SLOT(requestEngineTranslation(QString)));
    connect(rpc, SIGNAL(clearEngineRequested()), eng, SLOT(clearTranslation()));
    connect(rpc, SIGNAL(enableEngineRequested(bool)), eng, SLOT(setEnable(bool)));
    connect(rpc, SIGNAL(engineTranslationReceived(QString)), eng, SLOT(updateTranslation(QString)));
  }

  //if (auto p = AbstractEngine::getEngine())
  //  p->inject();
}

// EOF
