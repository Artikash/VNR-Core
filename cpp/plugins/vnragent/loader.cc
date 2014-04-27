// loader.cc
// 1/27/2013

#include "config.h"
#include "loader.h"
#include "driver/driver.h"
#include "qtembedded/applicationrunner.h"
#include "windbg/inject.h"
#include "windbg/util.h"
#include "ui/uihijack.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QTextCodec>

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

Driver *driver_;
QtEmbedded::ApplicationRunner *appRunner_;

} // unnamed namespace

// Loader

void Loader::initWithInstance(HINSTANCE hInstance)
{
  //::GetModuleFileNameW(hInstance, MODULE_PATH, MAX_PATH);

  QTextCodec *codec = QTextCodec::codecForName("UTF-8");
  QTextCodec::setCodecForCStrings(codec);
  QTextCodec::setCodecForTr(codec);

  auto app = ::createApplication_(hInstance);
  //::appLoader_ = new QtEmbedded::ApplicationLoader(app, QT_EVENTLOOP_INTERVAL);

  ::driver_ = new Driver;

  // Hijack UI threads
  {
    WinDbg::ThreadsSuspender suspendedThreads; // lock all threads
    Ui::overrideModules();
  }

  appRunner_ = new QtEmbedded::ApplicationRunner(app, QT_EVENTLOOP_INTERVAL);
  appRunner_->start();
  //qApp->exec(); // This might hang the game
}

void Loader::destroy()
{
  if (::appRunner_ && ::appRunner_->isActive())
    ::appRunner_->stop(); // this class is not deleted

  if (qApp) {
    qApp->quit();
    qApp->processEvents(); // might hang here
  }
}

// EOF
