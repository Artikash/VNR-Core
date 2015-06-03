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
  QString text; // already saved characters

  explicit DynamicShiftJISCodecPrivate(QTextCodec *codec)
    : codec(codec)
  {
    if (!codec)
      codec = QTextCodec::codecForName("Shift_JIS");
  }

  QByteArray encode(const QString &text);
  QString decode(const QByteArray &data) const;
};

QByteArray DynamicShiftJISCodecPrivate::encode(const QString &text)
{
  QByteArray ret;
  return ret;
}

QString DynamicShiftJISCodecPrivate::decode(const QByteArray &data) const
{
  QString ret;
  return ret;
}

/** Public class */

DynamicShiftJISCodec::DynamicShiftJISCodec(QTextCodec *codec)
  : d_(new D(codec))
{}

DynamicShiftJISCodec::~DynamicShiftJISCodec() { delete d_; }

int DynamicShiftJISCodec::size() const
{ return d_->text.size(); }

bool DynamicShiftJISCodec::isEmpty() const
{ return d_->text.isEmpty(); }

void DynamicShiftJISCodec::clear()
{ d_->text.clear(); }

QByteArray DynamicShiftJISCodec::encode(const QString &text) const
{ return d_->encode(text); }

QString DynamicShiftJISCodec::decode(const QByteArray &data) const
{ return d_->decode(data); }

// EOF
