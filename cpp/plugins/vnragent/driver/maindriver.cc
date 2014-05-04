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
  : Base(parent), settings(nullptr), rpc(nullptr), hijack(nullptr), win(nullptr), eng(nullptr)
{
  DOUT("enter");
  settings = new Settings(this);

  rpc = new RpcClient(this);
  {
    connect(rpc, SIGNAL(enableWindowTranslationRequested(bool)), settings, SLOT(setWindowTranslationEnabled(bool)));
    connect(rpc, SIGNAL(disconnected()), SLOT(onDisconnected()));
    connect(rpc, SIGNAL(detachRequested()), SLOT(unload()));
    connect(rpc, SIGNAL(enabledRequested(bool)), settings, SLOT(setEnabled(bool)));
    connect(rpc, SIGNAL(settingsReceived(QString)), settings, SLOT(load(QString)));

    connect(rpc, SIGNAL(enabledRequested(bool)), SLOT(onEnabledChanged()));
    connect(rpc, SIGNAL(enableEngineRequested(bool)), SLOT(onEnabledChanged()));
    connect(rpc, SIGNAL(enableWindowTranslationRequested(bool)), SLOT(onEnabledChanged()));
  }
  DOUT("leave");
}

void MainDriverPrivate::createHijackDriver()
{
  if (hijack)
    return;
  hijack = new HijackDriver(this);
}

void MainDriverPrivate::createWindowDriver()
{
  if (win)
    return;

  win = new WindowDriver(this);
  connect(win, SIGNAL(translationRequested(QString)), rpc, SLOT(requestWindowTranslation(QString)));

  connect(rpc, SIGNAL(clearWindowTranslationRequested()), win, SLOT(clearTranslation()));
  connect(rpc, SIGNAL(windowTranslationReceived(QString)), win, SLOT(updateTranslation(QString)));
}

void MainDriverPrivate::createEmbedDriver()
{
  if (eng)
    return;

  eng = new EmbedDriver(this);
  // Use queued connection to quarantine the engine thrread
  connect(eng, SIGNAL(textReceived(QString,qint64,int,bool)), rpc, SLOT(sendEngineText(QString,qint64,int,bool)),
      Qt::QueuedConnection);
  connect(eng, SIGNAL(textReceivedDelayed(QString,qint64,int,bool)), rpc, SLOT(sendEngineTextLater(QString,qint64,int,bool)),
      Qt::QueuedConnection);
  connect(eng, SIGNAL(engineNameChanged(QString)), rpc, SLOT(sendEngineName(QString)));
  connect(rpc, SIGNAL(clearEngineRequested()), eng, SLOT(clearTranslation()));
  connect(rpc, SIGNAL(enableEngineRequested(bool)), eng, SLOT(setEnable(bool)));
  connect(rpc, SIGNAL(engineTranslationReceived(QString,qint64,int)), eng, SLOT(updateTranslation(QString,qint64,int)),
      Qt::QueuedConnection);

  eng->inject();
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
  if (hijack) hijack->unload();
  if (eng) eng->unload();
  //DOUT("leave");
  WinDbg::unloadCurrentModule();
#endif // VNRAGENT_ENABLE_UNLOAD
}

void MainDriverPrivate::onEnabledChanged()
{
  if (!settings->isEnabled()) {
    if (eng)
      eng->setEnable(false);
    if (win)
      win->setEnable(false);
  } else {
    if (eng)
      eng->setEnable(settings->isEngineEnabled());
    else if (settings->isEngineEnabled()){
      createEmbedDriver();
      createHijackDriver();
    }
    if (win)
      win->setEnable(settings->isWindowTranslationEnabled());
    else if (settings->isWindowTranslationEnabled()){
      createWindowDriver();
      createHijackDriver();
    }
  }
}

// EOF
