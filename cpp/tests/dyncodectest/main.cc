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
  char data[] = "\x80\x10\x80\x40\x00\x00";
  QString t = QString::fromWCharArray((wchar_t *)data);
  QByteArray d = codec.encode(t);
  qDebug() << "encode:";
  qDebug() << t;
  qDebug() << t.size();
  qDebug() << d.size();
  qDebug() << d.toHex();
  QString s = codec.decode(d);
  qDebug() << "decode:";
  qDebug() << s;
  qDebug() << s.size();
  qDebug() << (t == s);
  return 0;
}
