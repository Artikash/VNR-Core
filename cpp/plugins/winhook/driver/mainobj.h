#ifndef MAINOBJ_H
#define MAINOBJ_H

// mainobj.h
// 2/1/2013 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QObject>
#include <qt_windows.h>

class MainObjectPrivate;
// Root object for all qobject
class MainObject : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(MainObject)
  SK_EXTEND_CLASS(MainObject, QObject)
  SK_DECLARE_PRIVATE(MainObjectPrivate)

  explicit MainObject(QObject *parent = nullptr);
public:
  ~MainObject();

  static Self *instance();
  static void init();
  static void destroy();

  void updateWindow(HWND hWnd);
  void updateContextMenu(HMENU hMenu, HWND hWnd);

  static void updateProcessWindows(DWORD processId = 0);
protected:
  static void updateThreadWindows(DWORD threadId = 0);
};

#endif // MAINOBJ_H
