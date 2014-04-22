#ifndef _READER_METACALL_H
#define _READER_METACALL_H
// reader/metacall.h
// 2/1/2013 jichi

#include "qtmetacall/metacallpropagator.h"
#include <QtCore/QObject>

class ReaderMetaCallPropagator : public QtMetaCall::MetaCallPropagator
{
  Q_OBJECT
  Q_DISABLE_COPY(ReaderMetaCallPropagator)
  SK_EXTEND_CLASS(ReaderMetaCallPropagator, QtMetaCall::MetaCallPropagator)

public:
  explicit ReaderMetaCallPropagator(QObject *parent = nullptr) : Base(parent) {}

signals:
  // - Control -

  void pingServer(int key); // client => server
  void q_pingServer(int key);

  void pingClient(int key); // server => client, for debug only
  void q_pingClient(int key);

  void callClient(const QString &cmd); // s=>c
  void q_callClient(const QString &cmd);

  // - Data -

  void updateServerData(const QString &json); // c=>s
  void q_updateServerData(const QString &json);

  void updateClientData(const QString &json); // s=>c
  void q_updateClientData(const QString &json);

  // - Messages

  void growlServerMessage(const QString &text); // c=>s
  void q_growlServerMessage(const QString &text);

  void growlServerWarning(const QString &text); // c=>s
  void q_growlServerWarning(const QString &text);

  void growlServerError(const QString &text); // c=>s
  void q_growlServerError(const QString &text);
};

#endif // _READER_METACALL_H
