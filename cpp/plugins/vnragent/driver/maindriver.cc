// maindriver.cc
// 2/1/2013 jichi

#include "driver/maindriver.h"
#include "driver/rpcclient.h"
#include "engine/enginedriver.h"
#include "hijack/hijackdriver.h"
#include "ui/uidriver.h"

/** Private class */

class MainDriverPrivate
{
public:
  RpcClient *rpc;
  HijackDriver *hijack;
  EngineDriver *eng;
  UiDriver *ui;

  explicit MainDriverPrivate(QObject *q);
};

MainDriverPrivate::MainDriverPrivate(QObject *q)
{
  rpc = new RpcClient(q);

  hijack = new HijackDriver(q);

  ui = new UiDriver(q); // TODO: Selective create ui only if enabled at server side, i.e. only called by rpc
  {
    QObject::connect(ui, SIGNAL(translationRequested(QString)), rpc, SLOT(requestUiTranslation(QString)));
    QObject::connect(rpc, SIGNAL(clearUiRequested()), ui, SLOT(clearTranslation()));
    QObject::connect(rpc, SIGNAL(enableUiRequested(bool)), ui, SLOT(setEnable(bool)));
    QObject::connect(rpc, SIGNAL(uiTranslationReceived(QString)), ui, SLOT(updateTranslation(QString)));
  }

  eng = new EngineDriver(q); // TODO: Selective create engine only if enabled at server side, i.e. only called by rpc {
  {
    // Use queued connection to quarantine the engine thrread
    QObject::connect(eng, SIGNAL(textReceived(QString,qint64,int,bool)), rpc, SLOT(sendEngineText(QString,qint64,int,bool)),
        Qt::QueuedConnection);
    QObject::connect(rpc, SIGNAL(clearEngineRequested()), eng, SLOT(clearTranslation()));
    QObject::connect(rpc, SIGNAL(enableEngineRequested(bool)), eng, SLOT(setEnable(bool)));
    QObject::connect(rpc, SIGNAL(engineTranslationReceived(QString,qint64,int)), eng, SLOT(updateTranslation(QString,qint64,int)),
        Qt::QueuedConnection);
  }
}


/** Public class */

MainDriver::MainDriver(QObject *parent) : Base(parent), d_(new D(this)) {}
MainDriver::~MainDriver() { delete d_; }

void MainDriver::quit() {}

// EOF
