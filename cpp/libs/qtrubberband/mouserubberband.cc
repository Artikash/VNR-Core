// mouserubberband.cc
// 8/21/2014 jichi
#include "qtrubberband/mouserubberband.h"
#include <QtGui/QApplication>

/** Private class */

class MouseRubberBandPrivate
{
public:
  bool pressed;
  int x, y;

  MouseRubberBandPrivate() : pressed(false) {} // x,y are not initialized
};

/** Public class */

MouseRubberBand::MouseRubberBand(QObject *parent) : Base(parent), d_(new D) {}
MouseRubberBand::~MouseRubberBand() { delete d_; }

bool MouseRubberBand::isPressed() const { return d_->pressed; }

void MouseRubberBand::press(int x, int y)
{
  setGeometry(x, y, 0, 0)
  d_->x = x;
  d_->y = y;
  d_->pressed = true;
  QApplication::setOverrideCursor(Qt::CrossCursor);
  show();
}

void MouseRubberBand::move(int x, int y)
{
  if (d_->pressed) {
    int x1 = qMin(x, d_->x),
        x2 = qMax(x, d_->x),
        y1 = qMin(y, d_->y),
        y2 = qMax(y, d_->y);
    setGeometry(x1, y1, x2 - x1, y2 - y1);
  }
}

void MouseRubberBand::release()
{
  cancel();
  QRect r = self.geometry();
  if (!r.isEmpty())
    emit selected(r.x(), r.y(), r.width(), r.height());
}

void MouseRubberBand::cancel()
{
  hide();
  if (QApplication::overrideCursor())
    QApplication::restoreOverrideCursor();
  d_->pressed = false;
}

// EOF
