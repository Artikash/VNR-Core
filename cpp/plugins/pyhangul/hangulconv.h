#ifndef HANGULCONV_H
#define HANGULCONV_H

// pyhangul.h
// 1/6/2015 jichi
// Qt is used instead of pure C++ to reduce memory copy of the returned containers.

#include "sakurakit/skglobal.h"
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QString>

class HangulHanjaConverter_p;
class HangulHanjaConverter
{
  SK_CLASS(HangulHanjaConverter)
  SK_DISABLE_COPY(HangulHanjaConverter)
  SK_DECLARE_PRIVATE(HangulHanjaConverter_p)

public:
  // Construction

  HangulHanjaConverter();
  ~HangulHanjaConverter();

  ///  Return the number of loaded rules
  int size() const;

  ///  Return whether the script has been loaded, thread-safe
  bool isEmpty() const;

  ///  Clear the loaded script
  void clear();

  ///  Add script from file
  bool loadFile(const QString &path);

  // Parse

  // Replace the characters according to the script, thread-safe
  QString convert(const QString &text) const;

  // Replace the characters according to the script, thread-safe
  QList<QList<QPair<QString, QString> > > parse(const QString &text) const;
};

#endif // HANGULCONV_H
