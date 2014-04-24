#ifndef _QTEMBEDDED_APPLICATIONLOADER_H
#define _QTEMBEDDED_APPLICATIONLOADER_H

// applicationloader.h
// 2/1/2013 jichi

#include "qtembedded/qtembedded.h"
#include "sakurakit/skglobal.h"
#include <QtCore/QString>
#include <QtGui/qwindowdefs.h>

QT_FORWARD_DECLARE_CLASS(QCoreApplication)

QTEMBEDDED_BEGIN_NAMESPACE

class ApplicationLoaderPrivate;
class ApplicationLoader
{
  SK_CLASS(ApplicationLoader)
  SK_DISABLE_COPY(ApplicationLoader)
  SK_DECLARE_PRIVATE(ApplicationLoaderPrivate)

public:
  // Default event loop timer interval in ms
  enum { DefaultEventLoopInterval = 100 };

  static QCoreApplication *createApplication(HINSTANCE hInstance = nullptr);

  ///**
  // *  Create an instance in the memory.
  // *  @param  interval  msecs to refresh the event loops
  // */
  //static Self *createInstance(int interval = DefaultEventLoopInterval);

  /////  Stop the event loop and delete the instance.
  //static void destroyInstance(Self *inst);

  /**
   *  App will be automatically deleted after destruction.
   *  @param  app
   *  @param  interval  msecs to refresh the event loops
   */
  explicit ApplicationLoader(QCoreApplication *app, int interval = DefaultEventLoopInterval);
  ~ApplicationLoader();

  int eventLoopInterval() const;

  void quit();
  void processEvents();

};

QTEMBEDDED_END_NAMESPACE

#endif // _QTEMBEDDED_APPLICATIONLOADER_H
