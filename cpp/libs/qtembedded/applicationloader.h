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
  /**
   *  App will be automatically deleted after destruction.
   *  @param  app
   *  @param  window
   *  @param  interval  msecs to refresh the event loops
   */
  ApplicationLoader(QCoreApplication *app, int interval);
  ~ApplicationLoader();

  void quit();

  void processEvents();
  static QCoreApplication *createApplication();
};

QTEMBEDDED_END_NAMESPACE

#endif // _QTEMBEDDED_APPLICATIONLOADER_H
