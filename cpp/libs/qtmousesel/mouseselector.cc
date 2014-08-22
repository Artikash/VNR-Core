// mouseselector.cc
// 8/21/2014 jichi
#include "qtmousesel/mouseselector.h"
#include "qtrubberband/mouserubberband.h"

#ifdef Q_OS_WIN
# include "mousehook/mousehook.h"
# include "winkey/winkey.h"
#else
# error "Windows only"
#endif // Q_OS_WIN

#include <boost/bind.hpp>

#define DEBUG "mouseselector"
#include "sakurakit/skdebug.h"

/** Private class */

class MouseSelectorPrivate
{
  typedef MouseSelectorPrivate Self;
  typedef MouseSelector Q;
  Q *q_;

public:
  // For mouse selection
  bool enabled;
  QWidget *parentWidget;
  MouseRubberBand *rb;

  void createRubberBand()
  {
    rb = new MouseRubberBand(QRubberBand::Rectangle, parentWidget);
    q_->connect(rb, SIGNAL(selected(int,int,int,int)), SIGNAL(selected(int,int,int,int)),
                Qt::QueuedConnection); // use queued connection to leave mouse event loop
  }

  // For mouse hook
  bool pressed;

//private:
  bool onMousePress(int x, int y, void *wid)
  {
    Q_UNUSED(wid);
    if (enabled //&& !pressed  // !pressed not checked in case sth is wrong at runtime
                && isPressAllowed()) {
      pressed = true;
      rb->press(x, y);
      DOUT("pass");
      return true;
    }
    return false;
  }

  bool onMouseMove(int x, int y, void *wid)
  {
    Q_UNUSED(wid);
    if (enabled && pressed)
      rb->move(x, y);
    return false;
  }

  bool onMouseRelease(int x, int y, void *wid)
  {
    Q_UNUSED(wid);
    if (enabled && pressed) {
      pressed = false;
      rb->move(x, y);
      rb->release();
      DOUT("pass");
      return true;
    }
    return false;
  }

  static bool isPressAllowed() { return WinKey::isKeyShiftPressed(); }

public:
  explicit MouseSelectorPrivate(Q *q)
    : q_(q), enabled(false), parentWidget(nullptr), rb(nullptr)
    , pressed(false)
  {
    ::mousehook_onlbuttondown(boost::bind(&Self::onMousePress, this, _1, _2, _3));
    ::mousehook_onlbuttonup(boost::bind(&Self::onMouseRelease, this, _1, _2, _3));
    ::mousehook_onmove(boost::bind(&Self::onMouseMove, this, _1, _2, _3));
  }

  ~MouseSelectorPrivate()
  {
    if (enabled)
      ::mousehook_stop();
    ::mousehook_onmove(mousehook_fun_null);
    ::mousehook_onlbuttondown(mousehook_fun_null);
    ::mousehook_onlbuttonup(mousehook_fun_null);
    if (rb)
      delete rb;
  }
};

/** Public class */

MouseSelector::MouseSelector(QObject *parent) : Base(parent), d_(new D(this)) {}
MouseSelector::~MouseSelector() { delete d_; }

//QWidget *MouseSelector::parentWidget() const { return d_->parentWidget; }
//void MouseSelector::setParentWidget(QWidget *v)
//{
//  if (d_->parentWidget != v) {
//    d_->parentWidget = v;
//    if (d_->rb)
//      d_->rb->setParent(v);
//  }
//}

bool MouseSelector::isEnabled() const
{ return d_->enabled; }

void MouseSelector::setEnabled(bool t)
{
  if (d_->enabled != t) {
    DOUT(t);
    if (t) {
      if (!d_->rb)
        d_->createRubberBand();
      d_->enabled = true;
      ::mousehook_start();
    } else {
      ::mousehook_stop();
      d_->enabled = false;
    }
  }
}

// EOF
