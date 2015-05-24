// main.cc
// 4/5/2014 jichi
#include <QtCore>

bool encodable(const QChar &c, QTextEncoder *encoder)
{
  if (!encoder || c.isNull())
    return false;
  //if (c.unicode() == '?')
  if (c.unicode() <= 127)
    return true;
  return encoder->fromUnicode(&c, 1) != "?";
}

int main()
{
  QString t = QString::fromStdWString(L"\u76ee");
  //QString t = QString::fromStdWString(L"\u899a");
  //QTextCodec *c = QTextCodec::codecForName("euc-kr");
  QTextCodec *c = QTextCodec::codecForName("latin1");
  qDebug() << c;
  //QTextEncoder *e = c->makeEncoder(QTextCodec::ConvertInvalidToNull);
  QTextEncoder *e = c->makeEncoder();
  QByteArray b = e->fromUnicode(t);
  qDebug() << t;
  qDebug() << b.size();
  qDebug() << QString(b);
  qDebug() << encodable(t[0], e);

  QChar ch(0x76ee);
  qDebug() << QTextCodec::codecForName("euc-kr")->makeEncoder()->fromUnicode(&ch, 1);
  return 0;
}
