#ifndef RPCCLI_P_H
#define RPCCLI_P_H
// rpccli_p.h
// 2/1/2013 jichi

#include "services/reader/metacall.h"
#include "wintimer/wintimer.h"
#include <QtCore/QObject>

class RpcClient;
class RpcClientPrivate;

class RpcPropagator : public ReaderMetaCallPropagator
{
  Q_OBJECT
  Q_DISABLE_COPY(RpcPropagator)
  SK_EXTEND_CLASS(RpcPropagator, ReaderMetaCallPropagator)
  friend class RpcClient;
  friend class RpcClientPrivate;

public:
  explicit RpcPropagator(QObject *parent = nullptr)
    : Base(parent)
  {
    connect(this, SIGNAL(q_pingServer(int)), SIGNAL(pingServer(int)), Qt::QueuedConnection);
    connect(this, SIGNAL(q_updateServerData(QString)), SIGNAL(updateServerData(QString)), Qt::QueuedConnection);

    connect(this, SIGNAL(q_growlServerMessage(QString)), SIGNAL(growlServerMessage(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(q_growlServerWarning(QString)), SIGNAL(growlServerWarning(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(q_growlServerError(QString)), SIGNAL(growlServerError(QString)), Qt::QueuedConnection);
  }
};

class RpcClientPrivate : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(RpcClientPrivate)
  SK_DECLARE_PUBLIC(RpcClient)
  SK_EXTEND_CLASS(RpcClientPrivate, QObject)

public:
  explicit RpcClientPrivate(Q *q);
  ~RpcClientPrivate();

  RpcPropagator *r;
  WinTimer reconnectTimer;

  enum { PORT = 6103 }; // must be consistent with the metacall port defined in reader.yaml
  bool start() { return r->startClient("127.0.0.1", PORT); }

  bool reconnect();

protected slots:
  void onCall(const QString &cmd);
};

#endif // RPCCLI_P_H

// EOF

/*
class RpcRouter : public MetaCallRouter
{
public:
  int convertSendMethodId(int value) override
  {
    switch (value) {
    case 6: return 16; // pingServer
    case 8: return 10; // pingClient
    case 10: return 14; // updateServerString
    default: return value;
    }
  }

  int convertReceiveMethodId(int value) override
  {
    switch (value) {
    case 12: return 12; // updateClientString
    default: return value;
    }
  }
};

*/
