#ifndef UIDRIVER_H
#define UIDRIVER_H

// uidriver.h
// 2/1/2013 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QObject>
#include <qt_windows.h>

class UiDriverPrivate;
class UiDriver : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(UiDriver)
  SK_EXTEND_CLASS(UiDriver, QObject)
  SK_DECLARE_PRIVATE(UiDriverPrivate)

public:
  static Self *instance();
  explicit UiDriver(QObject *parent = nullptr);
  ~UiDriver();

public:
  void updateWindow(HWND hWnd);
  void updateContextMenu(HMENU hMenu, HWND hWnd);

  static void updateProcessWindows(DWORD processId = 0);
protected:
  static void updateThreadWindows(DWORD threadId = 0);
};

#endif // UIDRIVER_H
