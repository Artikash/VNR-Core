// driver.cc
// 2/1/2013 jichi

#include "driver/driver.h"
#include "driver/driver_p.h"
#include "driver/rpccli.h"
#include "ui/uidriver.h"

/** Public class */

Driver::Driver() : d_(new D) {}
Driver::~Driver() { delete d_; }

void Driver::quit()
{
  // TODO
  //d_->rpc->close();
}

/** Private class */

DriverPrivate::DriverPrivate(QObject *parent)
  : Base(parent)
{
  rpc = new RpcClient(this);
  ui = new UiDriver(this); // TODO: Selective create ui only if enabled at server side, i.e. only called by rpc

  connect(rpc, SIGNAL(clearUiRequested()), ui, SLOT(clearTranslation()));
  connect(rpc, SIGNAL(enableUiRequested(bool)), ui, SLOT(setEnable(bool)));
  connect(rpc, SIGNAL(uiTranslationReceived(QString)), ui, SLOT(updateTranslation(QString)));
}

// EOF
