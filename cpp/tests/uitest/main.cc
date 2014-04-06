// main.cc
// 4/5/2014 jichi
#include "graphicseffect/graphicsgloweffect.h"
#include <QtGui>

static QGraphicsEffect *createEffect()
{
  auto e = new GraphicsGlowEffect;
  e->setColor(Qt::blue);
  e->setOffset(1, 1);
  e->setBlurRadius(8);;
  e->setBlurIntensity(1);
  e->setEnabled(true);
  return e;
}

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  auto w = new QTextEdit;

  w->setAttribute(Qt::WA_TranslucentBackground);
  w->setStyleSheet("background-color:transparent");
  w->setWindowFlags(Qt::FramelessWindowHint);

  auto font = w->font();
  font.setPointSize(36);
  font.setBold(true);
  w->setFont(font);

  //w->setTextColor(Qt::white);
  auto h = "<span style='color:white'>a very long text</span>";
  w->setHtml(h);

  auto e = createEffect();
  w->setGraphicsEffect(e);
  w->resize(400, 300);
  w->show();
  return a.exec();
}

// EOF
