#ifndef MSIME_JA_H
#define MSIME_JA_H

// msime_ja.h
// 4/1/2013 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QString>

class Msime_ja_p;
class Msime_ja
{
  SK_CLASS(Msime_ja)
  SK_DISABLE_COPY(Msime_ja)
  SK_DECLARE_PRIVATE(Msime_ja_p)

public:
  // - Types -

  enum Type { AnyType = 0, Hiragana = 1, Katagana = 2 }; // Type
  enum Flag {
    DefaultFlag = 0,
    Autocorrect = 1 // First convert kanji to kana, and then convert back
  }; // flags;

  // - Construction -

  Msime_ja();
  ~Msime_ja();

  bool isValid() const;

  // - Actions -

  QString toYomigana(const QString& text, int type = AnyType) const;
  QList<QPair<QString, QString> > toFurigana(const QString& text, int type = AnyType) const;
  QString toKanji(const QString &text, ulong flags = 0) const;
};

#endif // MSIME_JA_H
