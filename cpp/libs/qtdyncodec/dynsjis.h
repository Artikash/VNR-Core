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

  /**
   *  @param  text
   *  @param* dynamic  whether there are unencodable character
   *  @return  data
   */
  QByteArray encode(const QString &text, bool *dynamic = nullptr) const;

  /**
   *  @param  data
   *  @param* dynamic  whether there are undecodable character
   *  @return  text
   */
  QString decode(const QByteArray &data, bool *dynamic = nullptr) const;
};

#endif // DYNSJIS_H
