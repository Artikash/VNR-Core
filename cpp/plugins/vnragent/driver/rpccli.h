#ifndef RPCCLI_H
#define RPCCLI_H

// netman.h
// 2/1/2013 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QObject>

class RpcClientPrivate;
class RpcClient : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(RpcClient)
  SK_EXTEND_CLASS(RpcClient, QObject)
  SK_DECLARE_PRIVATE(RpcClientPrivate)

public:
  explicit RpcClient(QObject *parent = nullptr);
  ~RpcClient();
  bool isActive() const;

  // - API -
signals:
  void clearRequested();
  void enableRequested();
  void disableRequested();

  /**
   *  @param  json  {hash:text}
   */
  void dataReceived(QString json);
public slots:
  /**
   *  @param  json  {hash:context}
   */
  void sendData(const QString &json);

  void showMessage(const QString &message);
  void showWarning(const QString &message);
  void showError(const QString &message);
};

#endif // RPCCLI_H
