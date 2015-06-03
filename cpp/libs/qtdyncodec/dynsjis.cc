// dynsjis.cc
// 3/5/2015 jichi
// http://en.wikipedia.org/wiki/Shift_JIS
#include "qtdyncodec/dynsjis.h"
#include <QtCore/QTextCodec>

//#define SK_NO_QT
//#define DEBUG "dynsjis.cc"
//#include "sakurakit/skdebug.h"

/** Private class */

class DynamicShiftJISCodecPrivate
{
public:
  QTextCodec *codec; // sjis codec

  explicit DynamicShiftJISCodecPrivate(QTextCodec *codec)
    : codec(codec)
  {
    if (!codec)
      codec = QTextCodec::codecForName("Shift_JIS");
  }

  void clear()
  {
  }
};

/** Public class */

DynamicShiftJISCodec::DynamicShiftJISCodec(QTextCodec *codec)
  : d_(new D(codec))
{}

DynamicShiftJISCodec::~DynamicShiftJISCodec() { delete d_; }

int DynamicShiftJISCodec::size() const
{
  return 0;
}

bool DynamicShiftJISCodec::isEmpty() const
{
  return true;
}

void DynamicShiftJISCodec::clear()
{
}

QByteArray DynamicShiftJISCodec::encode(const QString &text) const
{
  QByteArray ret;
  return ret;
}

QString DynamicShiftJISCodec::decode(const QByteArray &data) const
{
  QString ret;
  return ret;
}


// EOF
