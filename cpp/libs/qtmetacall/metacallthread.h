#ifndef METACALLTHREAD_H
#define METACALLTHREAD_H

// metacallthread.h
// 4/27/2014 jichi

#include "qtmetacall/qtmetacall.h"
#include "sakurakit/skglobal.h"
#include <QtCore/QThread>

QTMETACALL_BEGIN_NAMESPACE

class MetaCallPropagator;
class MetaCallThreadPrivate;
/**
 *  All signals in this class will be propagated to the remote object.
 *  DO NOT USE LOCAL SIGNALS
 */
class MetaCallThread : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(MetaCallThread)
  SK_EXTEND_CLASS(MetaCallThread, QObject)
  SK_DECLARE_PRIVATE(MetaCallThreadPrivate)

public:
  explicit MetaCallThread(QObject *parent = nullptr);
  ~MetaCallThread();

  MetaCallPropagator *propagator() const;
  void setPropagator(MetaCallPropagator *value);

  // Following methods have the same interface with the propagator

  bool startServer(const QString &address, int port);
  bool startClient(const QString &address, int port);

  bool isServer() const; ///< Return true after startServer is invoked
  bool isClient() const; ///< Return true after startClient is invoked

  ///  Return true if startServer or startClient succeed
  bool isActive() const;

  ///  Return true after started, but is not guranteed to isActive
  bool isReady() const;

public slots:
  ///  Wait until ready. Only needed by client.
  void waitForReady() const;

  ///  Stop server/client
  void stop();
};

QTMETACALL_END_NAMESPACE

#endif // METACALLTHREAD_H
