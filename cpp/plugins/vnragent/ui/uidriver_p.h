#pragma once

// uidriver_p.h
// 2/1/2013 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QObject>
#include <qt_windows.h>

QT_FORWARD_DECLARE_CLASS(QTimer)

class UiManager;
//class UiDriver;
class UiDriverPrivate : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(UiDriverPrivate)
  //SK_DECLARE_PUBLIC(UiDriver)
  SK_EXTEND_CLASS(UiDriverPrivate, QObject)

  enum { RetransInterval = 500 }; // interval checking if new window is created
  enum { RehookInterval = 5000 }; // interval checking if new module/process is loaded
  QTimer *retransTimer,
         *rehookTimer;

public:
  bool enabled;
  UiManager *manager;

public:
  static Self *instance();
  explicit UiDriverPrivate(QObject *parent=nullptr);
  ~UiDriverPrivate();

  // Start refresh timers
  void start();

  void requestUpdateContextMenu(HMENU hMenu, HWND hWnd) // queued
  { emit updateContextMenuRequested(hMenu, hWnd); }

signals:
  void updateContextMenuRequested(void *hMenu, void *hWnd);
private slots:
  // Needed as HWMD and HMENU are not registered by Qt
  void onUpdateContextMenuRequested(void *hMenu, void *hWnd)
  { updateContextMenu((HMENU)hMenu, (HWND)hWnd); }

  void retrans();
  void rehook();

private:
  void updateAbstractWindow(HWND hWnd); // The type of the window is unknown

  bool updateStandardWindow(HWND hWnd, LPWSTR buffer, int bufferSize); // window
  void updateContextMenu(HMENU hMenu, HWND hWnd);
  bool updateListView(HWND hWnd, LPWSTR buffer, int bufferSize); // SysListView
  bool updateTabControl(HWND hWnd, LPWSTR buffer, int bufferSize); // SysTabControl
  bool updateMenu(HMENU hMenu, HWND hWnd, LPWSTR buffer, int bufferSize); // MenuItem
  bool updateTabView();

  static void repaintWindow(HWND hWnd);
  static void repaintMenuBar(HWND hWnd);

  static void updateProcessWindows(DWORD processId = 0);
  static void updateThreadWindows(DWORD threadId = 0);
};

// EOF
