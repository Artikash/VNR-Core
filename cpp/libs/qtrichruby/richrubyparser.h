#ifdef RICHRUBYPARSER_H
#define RICHRUBYPARSER_H

// richrubyparser.h
// 6/25/2015 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QString>

class RichRubyParserPrivate;
class RichRubyParser
{
  SK_CLASS(RichRubyParser)
  SK_DISABLE_COPY(RichRubyParser)
  SK_DECLARE_PRIVATE(RichRubyParserPrivate)

  // - Construction -
public:
  RichRubyParser();
  ~RichRubyParser();

  void setOpenChar(int v);
  int openChar() const;

  void setCloseChar(int v);
  int closeChar() const;

  void setSeparatorChar(int v);
  int separatorChar() const;

  QString renderTable(const QString &text) const; // render to html-like rich text

  bool containsRuby(const QString &text) const; // return if it contains ruby tag
  QString createRuby(const QString &rb, const QString &rt) const; // create one ruby tag
  QString removeRuby(const QString &text) const; // remove all ruby tags
};

#endif // RICHRUBYPARSER_H
