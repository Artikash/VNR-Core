// loader.cc
// 1/27/2013

#include "loader.h"
#include "global.h"
#include "driver/driver.h"
#include "qtembedded/applicationloader.h"
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
  ::appLoader_ = new QtEmbedded::ApplicationLoader(app, Global::EventLoopInterval);

  ::driver_ = new Driver;
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
