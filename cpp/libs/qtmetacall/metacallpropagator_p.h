#ifndef METACALLPROPAGATOR_P_H
#define METACALLPROPAGATOR_P_H

// metacallpropagator_p.h
// 4/9/2012 jichi

#include "qtmetacall/qtmetacall.h"
#include "sakurakit/skglobal.h"
#include <QtCore/QObject>

QT_FORWARD_DECLARE_CLASS(QTcpServer)
QT_FORWARD_DECLARE_CLASS(QTcpSocket)

QTMETACALL_BEGIN_NAMESPACE

class MetaCallSocketFilter;
class MetaCallPropagator;
class MetaCallPropagatorPrivate : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(MetaCallPropagatorPrivate)
  SK_EXTEND_CLASS(MetaCallPropagatorPrivate, QObject)
  SK_DECLARE_PUBLIC(MetaCallPropagator)

public:
  MetaCallSocketFilter *filter;
  QTcpServer *server; // for receiver
  QTcpSocket *socket; // for both sender and receiver

  explicit MetaCallPropagatorPrivate(Q *q);

  void createFilter(QObject *watched);

public slots:
  void dumpSocket() const; // for Debug only
  void serverAcceptsConnection();
};

QTMETACALL_END_NAMESPACE

#endif // METACALLPROPAGATOR_P_H
