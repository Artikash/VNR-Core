// driver.cc
// 2/1/2013 jichi

#include "global.h"
#include "driver/driver.h"
#include "driver/rpccli.h"
#include "ui/uidriver.h"

/** Private class */

class DriverPrivate
{
public:
  RpcClient *rpc;
  UiDriver *ui;

  explicit DriverPrivate(QObject *q);
  ~DriverPrivate();
};

DriverPrivate::DriverPrivate(QObject *q)
  : rpc(nullptr), ui(nullptr)
{
  rpc = new RpcClient(q);

  // TODO: Selective create ui only if enabled at server side, i.e. only called by rpc
  ui = new UiDriver(q);
}

DriverPrivate::~DriverPrivate()
{
  if (ui)
    delete ui;
  if (rpc)
    delete rpc;
}

/** Public class */

Driver::Driver() : d_(new D(this)) {}
Driver::~Driver() { delete d_; }

void Driver::quit()
{
  // TODO
  //d_->rpc->close();
}

// EOF
