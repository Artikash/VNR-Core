// main.cc
// 6/25/2015 jichi
//
// Measure text width in Qt
// http://doc.qt.io/qt-4.8/qfontmetrics.html
// http://stackoverflow.com/questions/1337523/measuring-text-width-in-qt
#include "qtrichruby/richrubyparser.h"
#include <QtGui>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  RichRubyParser p;

  auto w = new QTextEdit;
  w->setWordWrapMode(QTextOption::WordWrap);

  QString h = "first text very long {hello|ruby} what {hello|ruby}  again very longaweflong text another long long long text {ruby|a} later text that is very very long abcdawefaw awfawef awef";
  w->resize(200, 300);
  int contentWidth = w->contentsRect().width();
  QFont rbFont = w->font(),
        rtFont = w->font();
  rbFont.setBold(true);
  rtFont.setUnderline(true);
  qDebug() << p.removeRuby(h);
  h = p.renderTable(h, contentWidth, rbFont, rtFont);
  h.prepend(
    "<style type='text/css'>"
    ".rt { text-decoration: underline; }"
    "</style>"
  );
  qDebug() << h;
  w->setHtml(h);
  w->show();
  return a.exec();
}
