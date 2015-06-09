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
  QRegExp rx_("\\\\[0-9A-Z.\\[\\]]+");

  QString t = "hello\\C[0] 123";
  t.remove(rx_);
  qDebug() << t;
  return 0;
}
