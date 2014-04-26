// loader.cc
// 1/27/2013

#include "config.h"
#include "loader.h"
#include "driver/driver.h"
#include "qtembedded/applicationloader.h"
#include "windbg/inject.h"
#include "windbg/util.h"
#include "ui/uihijack.h"
#include <QtCore/QTextCodec>

// Global variables

namespace { // unnamed

Driver *driver_;
QtEmbedded::ApplicationLoader *appLoader_;

} // unnamed namespace

// Loader

void Loader::initWithInstance(HINSTANCE hInstance)
{
  //::GetModuleFileNameW(hInstance, MODULE_PATH, MAX_PATH);

  QTextCodec *codec = QTextCodec::codecForName("UTF-8");
  QTextCodec::setCodecForCStrings(codec);
  QTextCodec::setCodecForTr(codec);

  QCoreApplication *app = QtEmbedded::ApplicationLoader::createApplication(hInstance);
  ::appLoader_ = new QtEmbedded::ApplicationLoader(app, QT_EVENTLOOP_INTERVAL);

  ::driver_ = new Driver;

  // Hijack UI threads
  {
    WinDbg::ThreadsSuspender suspendedThreads; // lock all threads
    Ui::overrideModules();
  }
}

void Loader::destroy()
{
  if (::appLoader_) {
    if (::driver_)
      ::driver_->quit();

    ::appLoader_->quit();

    if (::driver_) {
      delete driver_;
      ::driver_ = nullptr;
    }

    delete ::appLoader_;
    ::appLoader_ = nullptr;
  }
}

// EOF
