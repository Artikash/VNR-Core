// maindriver.cc
// 2/1/2013 jichi

#include "config.h"
#include "driver/maindriver.h"
#include "driver/maindriver_p.h"
#include "driver/rpcclient.h"
#include "driver/settings.h"
#include "embed/embeddriver.h"
#include "hijack/hijackdriver.h"
#include "hijack/hijackdriver.h"
#include "window/windowdriver.h"
#include "windbg/unload.h"

#define DEBUG "maindriver"
#include "sakurakit/skdebug.h"

/** Public class */

MainDriver::MainDriver(QObject *parent)
  : Base(parent), d_(new D(this))
{
  connect(this, SIGNAL(deleteLaterRequested()), SLOT(deleteLater()), Qt::QueuedConnection);
}

MainDriver::~MainDriver() { delete d_; }

void MainDriver::requestDeleteLater() { emit deleteLaterRequested(); }

/** Private class */

MainDriverPrivate::MainDriverPrivate(QObject *parent)
  : Base(parent)
{
  DOUT("enter");
  settings = new Settings(this);

  rpc = new RpcClient(this);
  {
    connect(rpc, SIGNAL(enableWindowTranslationRequested(bool)), settings, SLOT(setWindowTranslationEnabled(bool)));
    connect(rpc, SIGNAL(disconnected()), SLOT(onDisconnected()));
    connect(rpc, SIGNAL(detachRequested()), SLOT(unload()));
  }

  hijack = new HijackDriver(this);

  win = new WindowDriver(this); // TODO: Selective create driver only if enabled at server side, i.e. only called by rpc
  {
    connect(win, SIGNAL(translationRequested(QString)), rpc, SLOT(requestWindowTranslation(QString)));

    connect(settings, SIGNAL(windowTranslationEnabledChanged(bool)), win, SLOT(setEnable(bool)));

    connect(rpc, SIGNAL(clearWindowTranslationRequested()), win, SLOT(clearTranslation()));
    connect(rpc, SIGNAL(windowTranslationReceived(QString)), win, SLOT(updateTranslation(QString)));
  }

  eng = new EmbedDriver(this); // TODO: Selective create engine only if enabled at server side, i.e. only called by rpc {
  {
    // Use queued connection to quarantine the engine thrread
    connect(eng, SIGNAL(textReceived(QString,qint64,int,bool)), rpc, SLOT(sendEngineText(QString,qint64,int,bool)),
        Qt::QueuedConnection);
    connect(eng, SIGNAL(textReceivedDelayed(QString,qint64,int,bool)), rpc, SLOT(sendEngineTextLater(QString,qint64,int,bool)),
        Qt::QueuedConnection);
    connect(rpc, SIGNAL(clearEngineRequested()), eng, SLOT(clearTranslation()));
    connect(rpc, SIGNAL(enableEngineRequested(bool)), eng, SLOT(setEnable(bool)));
    connect(rpc, SIGNAL(engineTranslationReceived(QString,qint64,int)), eng, SLOT(updateTranslation(QString,qint64,int)),
        Qt::QueuedConnection);
  }
  DOUT("leave");
}

void MainDriverPrivate::onDisconnected()
{
  settings->setWindowTranslationEnabled(false);
  //DOUT("pass");
  unload();
}

void MainDriverPrivate::unload()
{
#ifdef VNRAGENT_ENABLE_UNLOAD
  // Add contents to qDebug will crash the application while unload.
  //DOUT("enter");
  hijack->unload();
  eng->unload();
  //DOUT("leave");
  WinDbg::unloadCurrentModule();
#endif // VNRAGENT_ENABLE_UNLOAD
}

// EOF
