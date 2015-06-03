// dynsjis.cc
// 3/5/2015 jichi
// http://en.wikipedia.org/wiki/Shift_JIS
#include "qtdyncodec/dynsjis.h"
#include <QtCore/QTextCodec>
#include <QDebug>

#ifdef __clang__
# pragma GCC diagnostic ignored "-Wlogical-op-parentheses"
#endif // __clang__

//#define SK_NO_QT
//#define DEBUG "dynsjis.cc"
//#include "sakurakit/skdebug.h"

/** Private class */

class DynamicShiftJISCodecPrivate
{
public:
  // See: http://en.wikipedia.org/wiki/Shift_JIS
  enum { Capacity = // 7739
    (16 + 2) * 255 // first byte unused, 00 is skipped
    + (3 * 16 - 1) * (4 * 16 + 4 - 1) // second byte unused, 00 is skipped
  };

  QTextCodec *codec; // sjis codec
  std::wstring text; // already saved characters

  explicit DynamicShiftJISCodecPrivate(QTextCodec *codec)
    : codec(codec ? codec : QTextCodec::codecForName("Shift_JIS"))
  {}

  bool isFull() const { return text.size() == Capacity; }

  QByteArray encode(const wchar_t *text, size_t length);
  QString decode(const char *data, size_t length) const;

private:
  QByteArray encodeChar(wchar_t ch);
  wchar_t decodeChar(wchar_t ch1, wchar_t ch2) const;
};

// Encode

QByteArray DynamicShiftJISCodecPrivate::encode(const wchar_t *text, size_t length)
{
  QByteArray ret;
  for (size_t i = 0; i < length; i++) {
    wchar_t ch = text[i];
    if (ch <= 127)
      ret. push_back(ch);
    else {
      QChar qch(ch);
      QByteArray data = codec->fromUnicode(&qch, 1);
      if (data.size() == 1 && (!data[0] || data[0] == '?')) // failed to decode
        data = encodeChar(ch);
      ret.append(data);
    }
  }
  return ret;
}

QByteArray DynamicShiftJISCodecPrivate::encodeChar(wchar_t ch)
{
  QByteArray ret;
  size_t i = text.find(ch);
  if (i == std::wstring::npos) {
    if (isFull())
      return ret;
    i = text.size();
    text.push_back(ch);
  }
  if (i < 31 * (4 * 16 + 4 - 1)) {
    int v1 = i / (4 * 16 + 4 - 1) + 0x81,
        v2 = i % (4 * 16 + 4 - 1) + 1;
    if (v2 == 0x40)
      v2 = 0x7f;
    else if (v2 > 0x3f)
      v2 += 0xfd - 0x3f;
    ret.push_back(v1);
    ret.push_back(v2);
    return ret;
  }
  i -= 31 * (4 * 16 + 4 - 1);
  if (i < 16 * (4 * 16 + 4 - 1)) {
    int v1 = i / (4 * 16 + 4 - 1) + 0xe0,
        v2 = i % (4 * 16 + 4 - 1) + 1;
    if (v2 == 0x40)
      v2 = 0x7f;
    else if (v2 > 0x3f)
      v2 += 0xfd - 0x3f;
    ret.push_back(v1);
    ret.push_back(v2);
    return ret;
  }
  i -= 16 * (4 * 16 + 4 - 1);
  if (i < 255) {
    int v1 = 0x80,
        v2 = i % 255 + 1;
    ret.push_back(v1);
    ret.push_back(v2);
    return ret;
  }
  i -= 255;
  if (i < 255) {
    int v1 = 0xa0,
        v2 = i % 255 + 1;
    ret.push_back(v1);
    ret.push_back(v2);
    return ret;
  }
  i -= 255;
  if (i < 16 * 255) {
    int v1 = i / 255 + 0xf0,
        v2 = i % 255 + 1;
    ret.push_back(v1);
    ret.push_back(v2);
    return ret;
  }
  // This return should be unreachable
  return ret;
}

// Decode

QString DynamicShiftJISCodecPrivate::decode(const char *data, size_t length) const
{
  QString ret;
  for (size_t i = 0; i < length; i++) {
    quint8 ch = (quint8)data[i];
    if (ch <= 127)
      ret.push_back(ch);
    else if (ch >= 0xa1 && ch <= 0xdf) // size == 1
      ret.append(codec->toUnicode(data + 1, 1));
    else {
      if (i + 1 == length) // no enough character
        return ret;
      quint8 ch2 = (quint8)data[++i];
      if ((ch >= 0x81 && ch <= 0x9f || ch >= 0xe0 && ch <= 0xef)
          && (ch2 != 0x7f && ch2 >= 0x40 && ch2 <= 0xfc)) {
        ret.append(codec->toUnicode(data + i - 1, 2));
      }
      else if (wchar_t c = decodeChar(ch, ch2))
        ret.push_back(c);
    }
  }
  return ret;
}

wchar_t DynamicShiftJISCodecPrivate::decodeChar(wchar_t ch1, wchar_t ch2) const
{
  if (text.empty())
    return 0;
  size_t i = std::wstring::npos;
  if (ch1 >= 0x81 && ch1 <= 0x9f) {
    if (ch2 == 0x7f)
      ch2 = 0x40;
    else if (ch2 >= 0xfd)
      ch2 += 0x41 - 0xfd;
    i = (ch1 - 0x81) * (4 * 16 + 4 - 1) + ch2 - 1;
  } else if (ch1 >= 0xe0 && ch1 <= 0xef) {
    if (ch2 == 0x7f)
      ch2 = 0x40;
    else if (ch2 >= 0xfd)
      ch2 += 0x41 - 0xfd;
    i = (ch1 - 0xe0) * (4 * 16 + 4 - 1) + ch2 - 1
      + 31 * (4 * 16 + 4 - 1);
  } else if (ch1 == 0x80)
    i = ch2 - 1
      + 47 * (4 * 16 + 4 - 1);
  else if (ch1 == 0xa0)
    i = ch2 - 1
      + 47 * (4 * 16 + 4 - 1)
      + 255;
  else if (ch1 >= 0xf0 && ch1 <= 0xff)
    i = (ch1 - 0xf0) * 255 + ch2 - 1
      + 47 * (4 * 16 + 4 - 1)
      + 255 * 2;

  if (i != std::wstring::npos && i < text.size())
    return text[i];
  return 0;
}

/** Public class */

DynamicShiftJISCodec::DynamicShiftJISCodec(QTextCodec *codec)
  : d_(new D(codec))
{}

DynamicShiftJISCodec::~DynamicShiftJISCodec() { delete d_; }

int DynamicShiftJISCodec::capacity()
{ return D::Capacity; }

int DynamicShiftJISCodec::size() const
{ return d_->text.size(); }

bool DynamicShiftJISCodec::isEmpty() const
{ return d_->text.empty(); }

bool DynamicShiftJISCodec::isFull() const
{ return d_->isFull(); }

void DynamicShiftJISCodec::clear()
{ d_->text.clear(); }

QByteArray DynamicShiftJISCodec::encode(const QString &text) const
{
  if (!d_->codec)
    return text.toLocal8Bit();
  return d_->encode(reinterpret_cast<const wchar_t *>(text.utf16()), text.size());
}

QString DynamicShiftJISCodec::decode(const QByteArray &data) const
{
  if (!d_->codec)
    return QString::fromLocal8Bit(data);
  return d_->decode(data.constData(), data.size());
}

// EOF
