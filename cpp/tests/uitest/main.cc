// main.cc
// 4/5/2014 jichi
#include "graphicseffect/graphicsgloweffect.h"
#include <QtGui>

static QGraphicsEffect *createEffect()
{
  //auto e = new GraphicsGlowEffect();
  auto e = new QGraphicsBlurEffect();
  //e->setColor(Qt::red);
  //e->setOffset(1, 1);
  e->setBlurRadius(8);;
  //e->setBlurIntensity(1);
  e->setEnabled(true);
  return e;
}

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  auto t = "a very long text";
  auto w = new QLabel(t);
  auto e = createEffect();
  w->setGraphicsEffect(e);
  w->show();
  return a.exec();
}

// EOF
