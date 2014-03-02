#ifndef MAINOBJ_P_H
#define MAINOBJ_P_H

// mainobj_p.h
// 2/1/2013 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QObject>
#include <qt_windows.h>

class DataManager;
class RpcClient;
class MainObject;
class MainObjectPrivate : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(MainObjectPrivate)
  SK_DECLARE_PUBLIC(MainObject)
  SK_EXTEND_CLASS(MainObjectPrivate, QObject)

public:
  bool enabled;
  DataManager *dm;
  RpcClient *rpc;

public:
  explicit MainObjectPrivate(Q *q);

  bool updateWindow(HWND hWnd, LPWSTR buffer, int bufferSize); // window
  bool updateListView(HWND hWnd, LPWSTR buffer, int bufferSize); // SysListView
  bool updateTabControl(HWND hWnd, LPWSTR buffer, int bufferSize); // SysTabControl
  bool updateMenu(HMENU hMenu, HWND hWnd, LPWSTR buffer, int bufferSize); // MenuItem
  bool updateTabView();

  void setEnabled(bool value)
  {
    if (enabled != value) {
      enabled = value;
      emit enabledChanged(value);
    }
  }

signals:
  void enabledChanged(bool value);

public slots:
  void enable() { setEnabled(true); }
  void disable() { setEnabled(false); }
};

#endif // MAINOBJ_P_H
