// qtrichruby.cc
// 6/25/2015 jichi
#include "qtrichruby/qtrichruby.h"
#include <QtCore/QRegExp>


/** Private class */

class QtRichRubyPrivate
{
public:
  wchar_t openChar, closeChar, separatorChar;

  QtRichRubyPrivate()
    : openChar('{'), closeChar('}'), separatorChar('|') {}
};

/** Public class */

QtRichRuby::QtRichRuby() : d_(new D) {}
QtRichRuby::~QtRichRuby() { delete d_; }

int QtRichRuby::openChar() const { return d_->openChar; }
void QtRichRuby::setOpenChar(int v) { d_->openChar = v; }

int QtRichRuby::closeChar() const { return d_->closeChar; }
void QtRichRuby::setCloseChar(int v) { d_->closeChar = v; }

int QtRichRuby::separatorChar() const { return d_->separatorChar; }
void QtRichRuby::setSeparatorChar(int v) { d_->separatorChar = v; }

bool QtRichRuby::containsRuby(const QString &text) const
{
  return text.contains(d_->openChar)
      && text.contains(d_->closeChar)
      && text.contains(d_->separatorChar);
}

QString QtRichRuby::createRuby(const QString &rb, const QString &rt) const
{
  return QString()
      .append(d_->openChar)
      .append(rb)
      .append(d_->separatorChar)
      .append(rt)
      .append(d_->closeChar);
}

QString QtRichRuby::removeRuby(const QString &text) const
{
  return text;
}

QString QtRichRuby::render(const QString &text) const
{
  return text;
}



// EOF
