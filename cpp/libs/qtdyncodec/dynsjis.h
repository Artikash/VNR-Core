#ifndef DYNSJIS_H
#define DYNSJIS_H

// dynsjis.h
// 6/3/2014 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QString>

QT_FORWARD_DECLARE_CLASS(QTextCodec)

class DynamicShiftJISCodecPrivate;
class DynamicShiftJISCodec
{
  SK_CLASS(DynamicShiftJISCodec)
  SK_DISABLE_COPY(DynamicShiftJISCodec)
  SK_DECLARE_PRIVATE(DynamicShiftJISCodecPrivate)

  // - Construction -
public:
  explicit DynamicShiftJISCodec(QTextCodec *sjis = nullptr);
  ~DynamicShiftJISCodec();

  static int capacity(); // maximum allowed number of characters

  ///  Return the number of current characters
  int size() const;
  bool isEmpty() const;
  bool isFull() const;

  // Clear cached codec
  void clear();

  QByteArray encode(const QString &text) const;
  QString decode(const QByteArray &data) const;
};

#endif // DYNSJIS_H
