// applicationloader.cc
// 2/1/2013 jichi
//
// Motivations:
// http://stackoverflow.com/questions/2150488/using-a-qt-based-dll-in-a-non-qt-application
// http://stackoverflow.com/questions/1786438/qt-library-event-loop-problems

#include "qtembedded/applicationloader.h"
#ifdef WITH_LIB_WINTIMER
# include "wintimer/wintimer.h"
#else
# error "missing wintimer lib"
#endif // WITH_LIB_WINTIMER
#ifdef Q_OS_WIN
# include <qt_windows.h> // for GetModuleBase
#else
# error "windows only"
#endif // Q_OS_WIN
#include <QtCore/QCoreApplication>

//#define DEBUG "ApplicationLoader"
#include "sakurakit/skdebug.h"

QTEMBEDDED_BEGIN_NAMESPACE

/** Private class */

class ApplicationLoaderPrivate
{
  SK_CLASS(ApplicationLoaderPrivate)
  WinTimer t_;
public:
  QCoreApplication *app;

  ApplicationLoaderPrivate(): app(nullptr) {}
  ~ApplicationLoaderPrivate() { if (Q_LIKELY(app)) delete app; }

  void init(int interval)
  {
    t_.setInterval(interval);
    t_.setMethod(this, &Self::processEvents);
    t_.start();
  }

  void destroy()
  {
    t_.stop();
    if (Q_LIKELY(app)) {
      delete app;
      app = nullptr;
    }
  }

  void processEvents()
  {
    if (Q_LIKELY(app))
      app->processEvents(QEventLoop::AllEvents, t_.interval());
  }
};

/** Public class */

// - Construction -

ApplicationLoader::ApplicationLoader(QCoreApplication *app, int interval)
  : d_(new D)
{
  Q_ASSERT(app);
  d_->app = app;
  d_->init(interval);
}

ApplicationLoader::~ApplicationLoader()
{
  //d_->destroy();
  delete d_;
}

void ApplicationLoader::quit()
{ d_->destroy(); }

QCoreApplication *ApplicationLoader::createApplication()
{
  static int argc = 1;
  static char arg0[MAX_PATH * 2] = {0};
  static char *argv[] = { arg0, nullptr };
  if (Q_UNLIKELY(!*arg0))
    ::GetModuleFileNameA(nullptr, arg0, sizeof(arg0)/sizeof(*arg0));
  return new QCoreApplication(argc, argv);
}

void ApplicationLoader::processEvents() { d_->processEvents(); }

QTEMBEDDED_END_NAMESPACE

// EOF
