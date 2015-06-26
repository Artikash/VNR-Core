#ifndef QTRICHRUBY_H
#define QTRICHRUBY_H

// qtdynsjis.h
// 6/3/2015 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QString>

class QtRichRubyPrivate;
class QtRichRuby
{
  SK_CLASS(QtRichRuby)
  SK_DISABLE_COPY(QtRichRuby)
  SK_DECLARE_PRIVATE(QtRichRubyPrivate)

  // - Construction -
public:
  QtRichRuby();
  ~QtRichRuby();

  void setOpenChar(int v);
  int openChar() const;

  void setCloseChar(int v);
  int closeChar() const;

  void setSeparatorChar(int v);
  int separatorChar() const;

  QString render(const QString &text) const; // render to html-like rich text

  bool containsRuby(const QString &text) const; // return if it contains ruby tag
  QString createRuby(const QString &rb, const QString &rt) const; // create one ruby tag
  QString removeRuby(const QString &text) const; // remove all ruby tags
};

#endif // QTRICHRUBY_H
