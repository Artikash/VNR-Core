// maindriver_p.h
// 2/1/2013 jichi
#include "sakurakit/skglobal.h"
#include <QtCore/QObject>

class EngineDriver;
class HijackDriver;
class RpcClient;
class Settings;
class WindowDriver;
class MainDriverPrivate : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(MainDriverPrivate)
  SK_EXTEND_CLASS(MainDriverPrivate, QObject)

  Settings *settings;
  RpcClient *rpc;
  HijackDriver *hijack;
  EngineDriver *eng;
  WindowDriver *win;
public:
  explicit MainDriverPrivate(QObject *parent = nullptr);

private slots:
  void onDisconnected();
  void detach();
};

// EOF
