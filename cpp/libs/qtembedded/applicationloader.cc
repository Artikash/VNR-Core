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

  explicit ApplicationLoaderPrivate(QCoreApplication *app)
    : app(app) { Q_ASSERT(app); }

  // App is never deleted
  //~ApplicationLoaderPrivate()
  //{ delete app; }

  void startTimer(int interval)
  {
    t_.setInterval(interval);
    t_.setSingleShot(false); // repeat
    t_.setMethod(this, &Self::processEvents);
    t_.start();
  }

  int interval() const { return t_.interval(); }

  void processEvents() { app->processEvents(QEventLoop::AllEvents, t_.interval()); }

  void quit()
  {
    t_.stop();
    app->quit();
    app->processEvents(); // supposed to hang until all events are processed
  }
};

/** Public class */

// - Construction -

ApplicationLoader::ApplicationLoader(QCoreApplication *app, int interval)
  : d_(new D(app))
{
  Q_ASSERT(app);
  d_->startTimer(interval);
}

ApplicationLoader::~ApplicationLoader()
{
  //d_->destroy();
  delete d_;
}

void ApplicationLoader::quit()
{ d_->quit(); }

int ApplicationLoader::eventLoopInterval() const
{ return d_->interval(); }

QCoreApplication *ApplicationLoader::createApplication(HINSTANCE hInstance)
{
  static char arg0[MAX_PATH * 2]; // in case it is wchar

  static char *argv[] = { arg0, nullptr };
  static int argc = 1;

 ::GetModuleFileNameA(hInstance, arg0, sizeof(arg0)/sizeof(*arg0));
  return new QCoreApplication(argc, argv);
}

void ApplicationLoader::processEvents() { d_->processEvents(); }

QTEMBEDDED_END_NAMESPACE

// EOF

//ApplicationLoader *ApplicationLoader::createInstance(int interval)
//{
//  QCoreApplication *app = QCoreApplication::instance();
//  if (!app)
//    app = createApplication();
//  return new Self(app, interval);
//}

//void ApplicationLoader::destroyInstance(Self *inst)
//{
//  Q_ASSERT(inst);
//  if (inst) {
//    inst->quit();
//    delete inst;
//  }
//}
