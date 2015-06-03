// main.cc
// 4/5/2014 jichi
#include <QtCore>
#include "qtdyncodec/dynsjis.h"

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
  DynamicShiftJISCodec codec;
  QString t = QString::fromWCharArray(L"可爱");
  QByteArray d = codec.encode(t);
  qDebug() << "encode:";
  qDebug() << t;
  qDebug() << d.size();
  qDebug() << d.toHex();
  QString s = codec.decode(d);
  qDebug() << "decode:";
  qDebug() << s.size();
  qDebug() << s;
  qDebug() << (t == s);
  return 0;
}
