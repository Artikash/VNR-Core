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

  // For mouse hook
  bool pressed;

private:
  bool onMousePress(int x, int y, void *wid)
  {
    Q_UNUSED(wid);
    if (enabled && pressed && isPressAllowed()) {
      pressed = true;
      rb->press(x, y);
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
    ::mousehook_lbuttondown(boost::bind(&Self::onMousePress, this));
    ::mousehook_lbuttonup(boost::bind(&Self::onMouseRelease, this));
    ::mousehook_onmove(boost::bind(&Self::onMouseMove, this));
  }

  ~MouseSelectorPrivate()
  {
    if (enabled)
      ::mousehook_stop();
    ::mousehook_onmove(nullptr);
    ::mousehook_lbuttondown(nullptr);
    ::mousehook_lbuttonup(nullptr);
    if (rb)
      delete rb;
  }
};

/** Public class */

MouseSelector::MouseSelector(QObject *parent) : Base(parent), d_(new D(this)) {}
MouseSelector::~MouseSelector() { delete d_; }

QWidget *MouseSelector::parentWidget() const { return d_->parentWidget; }
void MouseSelector::setParentWidget(QWidget *v)
{
  if (d_->parentWidget != v) {
    d_->parentWidget = v;
    if (d_->rb)
      d_->rb->setParent(v);
  }
}

bool MouseSelector::isEnabled() const
{ return d_->enabled; }

void MouseSelector::setEnabled(bool t)
{
  if (d_->enabled != t) {
    if (t) {
      if (!t_->rb)
        t_->rb = new MouseRubberBand(QRubberBand::Rectangle, d_->parentWidget);
      d_->enabled = true;
      ::mousehook_start();
    } else {
      ::mousehook_stop();
      d_->enabled = false;
    }
  }
}

// EOF
