#ifndef METACALLTHREAD_P_H
#define METACALLTHREAD_P_H

// metacallthread_p.h
// 4/27/2014 jichi

#include "qtmetacall/qtmetacall.h"
#include "sakurakit/skglobal.h"
#include <QtCore/QObject>

QT_FORWARD_DECLARE_CLASS(QTcpServer)
QT_FORWARD_DECLARE_CLASS(QTcpSocket)

QTMETACALL_BEGIN_NAMESPACE

class MetaCallSocketFilter;
class MetaCallSocketObserver;
class MetaCallThread;
class MetaCallThreadPrivate : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(MetaCallThreadPrivate)
  SK_EXTEND_CLASS(MetaCallThreadPrivate, QObject)
  SK_DECLARE_PUBLIC(MetaCallThread)

public:
  MetaCallSocketFilter *filter;
  MetaCallSocketObserver *socketObserver;
  QTcpServer *server; // for receiver
  QTcpSocket *socket; // for both sender and receiver

  explicit MetaCallThreadPrivate(Q *q);

  void createFilter(QObject *watched);

  void connectSocketObserver();
public slots:
  void dumpSocket() const; // for Debug only
  void serverAcceptsConnection();
};

QTMETACALL_END_NAMESPACE

#endif // METACALLTHREAD_P_H
