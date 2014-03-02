#ifndef QAPPLICATIONLOADER_H
#define QAPPLICATIONLOADER_H

// qapplicationloader.h
// 2/1/2013 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QString>
#include <QtGui/qwindowdefs.h>

QT_FORWARD_DECLARE_CLASS(QCoreApplication)

class QApplicationLoaderPrivate;
class QApplicationLoader
{
  SK_CLASS(QApplicationLoader)
  SK_DISABLE_COPY(QApplicationLoader)
  SK_DECLARE_PRIVATE(QApplicationLoaderPrivate)

public:
  /**
   *  App will be automatically deleted after destruction.
   *  @param  app
   *  @param  window
   *  @param  interval  msecs to refresh the event loops
   */
  QApplicationLoader(QCoreApplication *app, int interval);
  ~QApplicationLoader();

  void quit();

  void processEvents();
  static QCoreApplication *createApplication();
};

#endif // QAPPLICATIONLOADER_H
