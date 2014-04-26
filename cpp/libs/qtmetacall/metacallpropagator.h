#ifndef METACALLPROPAGATOR_H
#define METACALLPROPAGATOR_H

// metacallpropagator.h
// 4/9/2012 jichi

#include "qtmetacall/qtmetacall.h"
#include "sakurakit/skglobal.h"
#include <QtCore/QObject>

QTMETACALL_BEGIN_NAMESPACE

class MetaCallRouter;
class MetaCallSocketObserver;
class MetaCallPropagatorPrivate;
/**
 *  All signals in this class will be propagated to the remote object.
 *  DO NOT USE LOCAL SIGNALS
 */
class MetaCallPropagator : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(MetaCallPropagator)
  SK_EXTEND_CLASS(MetaCallPropagator, QObject)
  SK_DECLARE_PRIVATE(MetaCallPropagatorPrivate)

public:
  explicit MetaCallPropagator(QObject *parent = nullptr);
  ~MetaCallPropagator();

  bool startServer(const QString &address, int port);
  bool startClient(const QString &address, int port);

  bool isServer() const; ///< Return true after startServer is invoked
  bool isClient() const; ///< Return true after startClient is invoked

  ///  Return true if startServer or startClient succeed
  bool isActive() const;

  ///  Return true after started, but is not guranteed to isActive
  bool isReady() const;

  ///  Used to convert method ID
  MetaCallRouter *router() const;
  void setRouter(MetaCallRouter *value);

  ///  Used to receive local signals. This class will NOT take the ownership.
  MetaCallSocketObserver *socketObserver() const;
  void setSocketObserver(MetaCallSocketObserver *value);

// Local signals has no effect!
//signals:
  //void connected();
  //void disconnected();
  //void socketError();

public slots:
  ///  Wait until ready. Only needed by client.
  void waitForReady() const;

  ///  Stop server/client
  void stop();
};

QTMETACALL_END_NAMESPACE

#endif // METACALLPROPAGATOR_H
