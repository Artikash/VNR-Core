// richrubyparser.cc
// 6/25/2015 jichi
#include "qtrichruby/richrubyparser.h"
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <functional>

/** Private class */

class RichRubyParserPrivate
{
public:
  wchar_t openChar, closeChar, splitChar;

  RichRubyParserPrivate()
    : openChar('{'), closeChar('}'), splitChar('|') {}

  typedef std::function<bool (const QString &rb, const QString &rt)> ruby_fun_t; // return if stop iteration
  void iterRuby(const QString &text,  const ruby_fun_t &fun) const;

  void removeRuby(QString &text) const;
};

void RichRubyParserPrivate::iterRuby(const QString &text,  const ruby_fun_t &fun) const
{
  QString rb, rt, plainText;
  bool rubyOpenFound = false,
       rubySepFound = false;
  foreach (const QChar &ch, text) {
    auto u = ch.unicode();
    if (u == openChar) {
      if (!plainText.isEmpty()) {
        if (!fun(plainText, rt))
          return;
        plainText.clear();
      }
      rubyOpenFound = true;
    } else if (u == splitChar) {
      if (rubyOpenFound)
        rubySepFound = true;
    } else if (u == closeChar) {
      if (rubyOpenFound) {
        if ((!rb.isEmpty() || !rt.isEmpty()) && !fun(rb, rt))
           return;
        rubySepFound = rubyOpenFound = false;
        rb.clear();
        rt.clear();
      }
    } else
      (!rubyOpenFound ? plainText : rubySepFound ? rt : rb).push_back(ch);
  }
  if (!plainText.isEmpty())
    rb = plainText;
  if (!rb.isEmpty() || !rt.isEmpty())
    fun(rb, rt);
}


void RichRubyParserPrivate::removeRuby(QString &ret) const
{
  for (int pos = ret.indexOf(openChar); pos != -1; pos = ret.indexOf(openChar, pos)) {
    int splitPos = ret.indexOf(splitChar, pos);
    if (splitPos == -1)
      return;
    int closePos = ret.indexOf(closeChar, splitPos);
    if (closePos == -1)
      return;
    ret.remove(closePos, 1);
    ret.remove(pos, splitPos - pos + 1);
    pos += closePos - splitPos - 1;
  }
  return;
}

/** Public class */

RichRubyParser::RichRubyParser() : d_(new D) {}
RichRubyParser::~RichRubyParser() { delete d_; }

int RichRubyParser::openChar() const { return d_->openChar; }
void RichRubyParser::setOpenChar(int v) { d_->openChar = v; }

int RichRubyParser::closeChar() const { return d_->closeChar; }
void RichRubyParser::setCloseChar(int v) { d_->closeChar = v; }

int RichRubyParser::splitChar() const { return d_->splitChar; }
void RichRubyParser::setSplitChar(int v) { d_->splitChar = v; }


bool RichRubyParser::containsRuby(const QString &text) const
{
  int pos = text.indexOf(d_->openChar);
  return pos != -1
      && (pos = text.indexOf(d_->closeChar, pos)) != -1
      && (pos = text.indexOf(d_->splitChar, pos)) != -1;
}

QString RichRubyParser::createRuby(const QString &rb, const QString &rt) const
{
  return QString()
      .append(d_->openChar)
      .append(rb)
      .append(d_->splitChar)
      .append(rt)
      .append(d_->closeChar);
}

QString RichRubyParser::removeRuby(const QString &text) const
{
  if (!containsRuby((text)))
    return text;
  QString ret = text;
  d_->removeRuby(ret);
  return ret;
}

QString RichRubyParser::renderTable(const QString &text, int width, const QFont &rbFont, const QFont &rtFont, int cellSpan) const
{
  if (!containsRuby((text)))
    return text;

  QString ret,
          tr_rb,
          tr_rt;
  QFontMetrics rbfm(rbFont),
               rtfm(rtFont);
  int tableWidth = 0;
  auto fun = [&](const QString &rb, const QString &rt) -> bool {
    if (rt.isEmpty() && ret.isEmpty() && rb == text) {
      ret = text;
      return false;
    }
    int cellWidth = qMax(
      rb.isEmpty() ? 0 : rbfm.width(rb),
      rt.isEmpty() ? 0 : rbfm.width(rt)
    );
    if (tableWidth + cellWidth + cellSpan > width // reduce table here
        && (!tr_rb.isEmpty() || !tr_rt.isEmpty())) {
      ret.append("<table>")
         .append("<tr class='rb'>").append(tr_rt).append("</tr>")
         .append("<tr class='rt'>").append(tr_rb).append("</tr>")
         .append("</table>");
      tr_rb.clear();
      tr_rt.clear();
      tableWidth = 0;
    }
    tableWidth += cellWidth + cellSpan;
    if (rb.isEmpty())
      tr_rb.append("<td/>");
    else
      tr_rb.append("<td>")
           .append(rb)
           .append("</td>");
    if (rt.isEmpty())
      tr_rt.append("<td/>");
    else
      tr_rt.append("<td>")
           .append(rt)
           .append("</td>");
    return true;
  };
  d_->iterRuby(text, fun);
  if (!tr_rb.isEmpty() || !tr_rt.isEmpty())
    ret.append("<table>")
       .append("<tr class='rb'>").append(tr_rt).append("</tr>")
       .append("<tr class='rt'>").append(tr_rb).append("</tr>")
       .append("</table>");
  return ret;
}

// EOF
