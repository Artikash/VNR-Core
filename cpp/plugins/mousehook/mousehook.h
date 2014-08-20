#pragma once

// mousehook.h
// 11/26/2011

#include "mousehook_config.h"
#include "sakurakit/skglobal.h"
#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(QObject)

class MouseHookPrivate;
///  Singleton class. Only one instance is allowed.
class MouseHook
{
  SK_CLASS(MouseHook)
  SK_DISABLE_COPY(MouseHook)
  SK_DECLARE_PRIVATE(MouseHookPrivate)

public:
  MouseHook();   ///< \internal
  ~MouseHook();  ///< \internal

  // Start/stop
  MOUSEHOOKAPI bool isActive() const;
  MOUSEHOOKAPI void start();       ///< Start getting the winId that the user clicked
  MOUSEHOOKAPI void stop();        ///< Stop hook service

  // Event listener
  MOUSEHOOKAPI QObject *eventListener() const;
  MOUSEHOOKAPI void setEventListener(QObject *listener);
};

// EOF
