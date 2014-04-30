// loader.cc
// 1/27/2013

#include "config.h"
#include "loader.h"
#include "driver/driver.h"
#include "windbg/inject.h"
#include "windbg/util.h"
#include "ui/uihijack.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QTextCodec>

#ifdef VNRAGENT_ENABLE_APPRUNNER
#include "qtembedded/applicationrunner.h"
#endif // VNRAGENT_ENABLE_APPRUNNER

#ifdef VNRAGENT_DEBUG
# include "util/msghandler.h"
#endif // VNRAGENT_DEBUG

// Global variables

namespace { // unnamed

QCoreApplication *createApplication_(HINSTANCE hInstance)
{
  static char arg0[MAX_PATH * 2]; // in case it is wchar
  static char *argv[] = { arg0, nullptr };
  static int argc = 1;
  ::GetModuleFileNameA(hInstance, arg0, sizeof(arg0)/sizeof(*arg0));
  return new QCoreApplication(argc, argv);
}

// Persistent data
Driver *driver_;

#ifdef VNRAGENT_ENABLE_APPRUNNER
QtEmbedded::ApplicationRunner *appRunner_;
#endif // VNRAGENT_ENABLE_APPRUNNER

} // unnamed namespace

// Loader

void Loader::initWithInstance(HINSTANCE hInstance)
{
  //::GetModuleFileNameW(hInstance, MODULE_PATH, MAX_PATH);

  QTextCodec *codec = QTextCodec::codecForName("UTF-8");
  QTextCodec::setCodecForCStrings(codec);
  QTextCodec::setCodecForTr(codec);

  ::createApplication_(hInstance);

#ifdef VNRAGENT_DEBUG
  Util::installDebugMsgHandler();
#endif // VNRAGENT_DEBUG

  ::driver_ = new Driver;

  // Hijack UI threads
  {
    WinDbg::ThreadsSuspender suspendedThreads; // lock all threads
    Ui::overrideModules();
  }

#ifdef VNRAGENT_ENABLE_APPRUNNER
  ::appRunner_ = new QtEmbedded::ApplicationRunner(qApp, QT_EVENTLOOP_INTERVAL);
  ::appRunner_->start();
#else
  qApp->exec(); // block here
#endif // VNRAGENT_ENABLE_APPRUNNER
}

void Loader::destroy()
{
  if (::driver_) {
    ::driver_->quit();
    //delete ::driver_; // Driver is never deleted.
  }
#ifdef VNRAGENT_ENABLE_APPRUNNER
  if (::appRunner_ && ::appRunner_->isActive())
    ::appRunner_->stop(); // this class is not deleted
#endif // VNRAGENT_ENABLE_APPRUNNER
  if (qApp) {
    qApp->quit();
    qApp->processEvents(); // might hang here
  }
}

// EOF
