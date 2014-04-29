// socketpack.cc
// 4/29/2014 jichi
#include "qtsocketsvc/socketpack.h"
#include <QtCore/QTextCodec>

#define STRING_ENCODING "UTF-8"

#define DEBUG "socketpack"
#include "sakurakit/skdebug.h"

//QTSS_BEGIN_NAMESPACE

QByteArray SocketService::packStringList(const QStringList &l, const char *encoding)
{
  QByteArray head, body;
  head.append(packInt32(l.size()));
  QTextCodec *codec = QTextCodec::codecForName(encoding);
  foreach (const QString &s, l) {
    QByteArray data = codec->fromUnicode(s);
    head.append(packInt32(data.size()));
    body.append(data);
  }

  //return head + body
  return head.append(body);
}

QStringList SocketService::unpackStringList(const QByteArray &data, const char *encoding)
{
  const int dataSize = data.size(); // never modified in this function
  if (dataSize < Int32Size) {
    DOUT("insufficient list size");
    return QStringList();
  }
  int headOffset = 0;
  int count = unpackInt32(data, headOffset); headOffset += Int32Size;
  if (Q_UNLIKELY(count == 0)) {
    DOUT("empty list");
    return QStringList();
  }
  if (Q_UNLIKELY(count < 0)) {
    DOUT("negative count");
    return QStringList();
  }

  int bodyOffset = headOffset + count * Int32Size;
  if (Q_UNLIKELY(bodyOffset > dataSize)) {
    DOUT("insufficient header size");
    return QStringList();
  }

  const char *p = data.constData();

  QTextCodec *codec = QTextCodec::codecForName(encoding);
  QStringList ret;
  for (int i = 0; i < count; i++) {
    int size = unpackInt32(data, headOffset); headOffset += Int32Size;
    if (Q_UNLIKELY(size < 0)) {
      DOUT("insufficient body size");
      return QStringList();
    } else if (size == 0)
      ret.append(QString());
    // size > 0
    else if (Q_UNLIKELY(bodyOffset + size > dataSize)) {
      DOUT("insufficient data size");
      return QStringList();
    } else {
      QString s = codec->toUnicode(p + bodyOffset, size); bodyOffset += size;
      ret.append(s);
    }
  }
  return ret;
}

//QTSS_END_NAMESPACE

// EOF

/*
namespace { // unnamed

// Force using UTF-8
QTextCodec *codec_ = QTextCodec::codecForName(STRING_ENCODING);

//inline QString toUnicode(const QByteArray &data) { return codec_->toUnicode(data); }
inline QString toUnicode(const char *data, int len) { return codec_->toUnicode(data, len); }
inline QByteArray fromUnicode(const QString &str) { return codec_->fromUnicode(str); }

#define packString(...)     fromUnicode(__VA_ARGS__)
#define unpackString(...)   toUnicode(__VA_ARGS__)

} // unnamed namespace
*/
