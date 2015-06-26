// richrubyparser.cc
// 6/25/2015 jichi
#include "qtrichruby/richrubyparser.h"
#include <QtCore/QRegExp>

/** Private class */

class RichRubyParserPrivate
{
public:
  wchar_t openChar, closeChar, separatorChar;

  RichRubyParserPrivate()
    : openChar('{'), closeChar('}'), separatorChar('|') {}
};

/** Public class */

RichRubyParser::RichRubyParser() : d_(new D) {}
RichRubyParser::~RichRubyParser() { delete d_; }

int RichRubyParser::openChar() const { return d_->openChar; }
void RichRubyParser::setOpenChar(int v) { d_->openChar = v; }

int RichRubyParser::closeChar() const { return d_->closeChar; }
void RichRubyParser::setCloseChar(int v) { d_->closeChar = v; }

int RichRubyParser::separatorChar() const { return d_->separatorChar; }
void RichRubyParser::setSeparatorChar(int v) { d_->separatorChar = v; }

bool RichRubyParser::containsRuby(const QString &text) const
{
  return text.contains(d_->openChar)
      && text.contains(d_->closeChar)
      && text.contains(d_->separatorChar);
}

QString RichRubyParser::createRuby(const QString &rb, const QString &rt) const
{
  return QString()
      .append(d_->openChar)
      .append(rb)
      .append(d_->separatorChar)
      .append(rt)
      .append(d_->closeChar);
}

QString RichRubyParser::removeRuby(const QString &text) const
{
  return text;
}

QString RichRubyParser::renderTable(const QString &text) const
{
  return text;
}

// EOF
