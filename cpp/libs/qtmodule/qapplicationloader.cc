// qapplicationloader.cc
// 2/1/2013 jichi
//
// Motivations:
// http://stackoverflow.com/questions/2150488/using-a-qt-based-dll-in-a-non-qt-application
// http://stackoverflow.com/questions/1786438/qt-library-event-loop-problems

#include "qtmodule/qapplicationloader.h"
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

//#define DEBUG "qapplicationloader"
#include "sakurakit/skdebug.h"

/** Private class */

class QApplicationLoaderPrivate
{
  SK_CLASS(QApplicationLoaderPrivate)
  WinTimer t_;
public:
  QCoreApplication *app;

  QApplicationLoaderPrivate(): app(nullptr) {}
  ~QApplicationLoaderPrivate() { if (app) delete app; }

  void init(int interval)
  {
    t_.setInterval(interval);
    t_.setMethod(this, &Self::processEvents);
    t_.start();
  }

  void destroy()
  {
    t_.stop();
    if (app) {
      delete app;
      app = nullptr;
    }
  }

  void processEvents()
  {
    if (app)
      app->processEvents(QEventLoop::AllEvents, t_.interval());
  }
};

/** Public class */

// - Construction -

QApplicationLoader::QApplicationLoader(QCoreApplication *app, int interval)
  : d_(new D)
{
  Q_ASSERT(app);
  d_->app = app;
  d_->init(interval);
}

QApplicationLoader::~QApplicationLoader()
{
  //d_->destroy();
  delete d_;
}

void QApplicationLoader::quit()
{ d_->destroy(); }

QCoreApplication *QApplicationLoader::createApplication()
{
  static int argc = 1;
  static char arg0[MAX_PATH * 2] = {0};
  static char *argv[] = { arg0, nullptr };
  if (!*arg0)
    ::GetModuleFileNameA(nullptr, arg0, sizeof(arg0)/sizeof(*arg0));
  return new QCoreApplication(argc, argv);
}

void QApplicationLoader::processEvents()
{ d_->processEvents(); }

// EOF
