// mousehook.cc
// 11/26/2011

#include "mousehook.h"
#include "mousehook_p.h"
#include "qtwin/qtwin.h"
#include <QtGui>
#include <qt_windows.h>

#define DEBUG "mousehook"
#include "qtx/qxdebug.h"

#ifndef MOUSEHOOK_DLL_NAME
# error "require DLL name"
#endif
#define MOUSEHOOK_MODULE_NAME        MOUSEHOOK_DLL_NAME

#define HOOKMAN  MouseHook::globalInstance()

#include <qt_windows.h>

class QObject;

class MouseHookPrivate
{
public:
  HHOOK hook;
  bool windowPosEnabled;
  QObject *listener;

public:
  MouseHookPrivate() : hook(nullptr), windowPosEnabled(false), listener(nullptr) { }
};

// - Helper -

namespace { namespace detail {

  inline QPoint POINT2QPoint(const POINT &pt)
  { return QPoint((int)pt.x, (int)pt.y); }

  inline POINT QPoint2POINT(const QPoint &pos)
  { POINT ret = { pos.x(), pos.y() }; return ret; }

  LRESULT CALLBACK
  MouseProc(__in int nCode, __in WPARAM wparam, __in LPARAM lparam)
  {
    #define NEXT ::CallNextHookEx(hHook, nCode, wparam, lparam)
    HHOOK hHook = (HHOOK)HOOKMAN->hook();
    Q_ASSERT(hHook);
    if (!hHook)
      return 0;

    // nCode can only be HC_ACTION(0) or HC_NOREMOVE (3)
    // See: http://msdn.microsoft.com/en-us/library/ms644988(v=vs.85).aspx
    if (nCode < 0 || !HOOKMAN->isActive())
      return NEXT;

    QEvent::Type type;
    switch (wparam) {
    case WM_MOUSEMOVE: type = QEvent::MouseMove; break;
    default: return NEXT;
    }

    LPMOUSEHOOKSTRUCT lpMouseEvent = (LPMOUSEHOOKSTRUCT)lparam;
    Q_ASSERT(lpMouseEvent);
    QPoint globalPos = POINT2QPoint(lpMouseEvent->pt);
    QPoint pos = globalPos;

    if (HOOKMAN->isWindowPosEnabled()) {
      HWND hwnd = lpMouseEvent->hwnd;
      if (hwnd) {
        RECT rect;
        if (::GetWindowRect(hwnd, &rect))
          pos -= QPoint(rect.left, rect.top);
      }
    }

    QObject *listener = HOOKMAN->eventListener();
    if (listener) {
      QMouseEvent e(type, pos, globalPos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
      QCoreApplication::sendEvent(listener, &e);

      // Always return next.
      //if (e.isAccepted())
      //  return 0;
    }

    return NEXT;
    #undef NEXT
  }

} } // anonymouss detail

// - Constructions -
//
MouseHook::MouseHook()
{ d_ = new Private; }

MouseHook::~MouseHook()
{
  stop();
  delete d_;
}

// - Properties -

void*
MouseHook::hook() const
{ return d_->hook; }

bool
MouseHook::isWindowPosEnabled() const
{ return d_->windowPosEnabled; }

void
MouseHook::setWindowPosEnabled(bool enabled)
{ d_->windowPosEnabled = enabled; }

QObject*
MouseHook::eventListener() const
{ return d_->listener; }

void
MouseHook::setEventListener(QObject *obj)
{ d_->listener = obj; }

// - Slots -

bool
MouseHook::isActive() const
{ return hook(); }

void
MouseHook::start()
{
  static HINSTANCE hInstance = 0;

  if (d_->hook)
    return;

  if (!hInstance) {
    DOUT("start: DLL name:" << MOUSEHOOK_MODULE_NAME);
    hInstance = ::GetModuleHandle(MOUSEHOOK_MODULE_NAME);

    Q_ASSERT(hInstance);
    if (!hInstance) // This mostly happens when dll is not available.
      return;
  }

  // Global mode
  d_->hook = ::SetWindowsHookEx(WH_MOUSE_LL, detail::MouseProc, hInstance, 0);
  DOUT("start: started");
}

void
MouseHook::stop()
{
  if (d_->hook) {
    ::UnhookWindowsHookEx((HHOOK)d_->hook);
    d_->hook = 0;
    DOUT("stop: stoppped");
  }
}

// EOF
