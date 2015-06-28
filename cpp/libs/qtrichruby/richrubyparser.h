#ifndef RICHRUBYPARSER_H
#define RICHRUBYPARSER_H

// richrubyparser.h
// 6/25/2015 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QString>

QT_FORWARD_DECLARE_CLASS(QFontMetrics)
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

  void setSplitChar(int v);
  int splitChar() const;

  bool containsRuby(const QString &text) const; // return if it contains ruby tag
  QString createRuby(const QString &rb, const QString &rt) const; // create one ruby tag
  QString removeRuby(const QString &text) const; // remove all ruby tags

  // render to html-like rich text
  QString renderToHtmlTable(const QString &text, int width, const QFontMetrics &rbFont, const QFontMetrics &rtFont, int cellSpace = 1, bool wordWrap = true) const;
  QString renderToPlainText(const QString &text) const;
};

#endif // RICHRUBYPARSER_H
