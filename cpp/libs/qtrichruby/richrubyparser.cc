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

  static QString partition(const QString &text, int width, const QFontMetrics &font);
};

QString RichRubyParserPrivate::partition(const QString &text, int width, const QFontMetrics &font)
{
  QString ret;
  if (width <= 0 || text.isEmpty())
    return ret;
  int retWidth = 0;
  foreach (const QChar &ch, text) {
    retWidth += font.width(ch);
    if (retWidth > width)
      break;
    ret.push_back(ch);
  }
  return ret;
}

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

QString RichRubyParser::renderTable(const QString &text, int width, const QFontMetrics &rbFont, const QFontMetrics &rtFont, int cellSpace) const
{
  if (!containsRuby((text)))
    return text;

  QString ret,
          tr_rb,
          tr_rt;
  int tableWidth = 0;

  auto reduce = [&]() {
    ret.append("<table>")
       .append("<tr class='rb'>").append(tr_rt).append("</tr>")
       .append("<tr class='rt'>").append(tr_rb).append("</tr>")
       .append("</table>");
  };

  d_->iterRuby(text, [&](const QString &_rb, const QString &rt) -> bool {
    QString rb = _rb;
    if (rt.isEmpty() && ret.isEmpty() && rb == text) {
      ret = text;
      return false;
    }
    int rbWidth =  rb.isEmpty() ? 0 : rbFont.width(rb),
        rtWidth =  rt.isEmpty() ? 0 : rtFont.width(rt),
        cellWidth = qMax(rbWidth, rtWidth);
    if (width > 0 && tableWidth < width - cellSpace && tableWidth + cellWidth > width - cellSpace && rb.size() > 1) { // split very long text
      QString left = D::partition(rb, width - cellSpace - tableWidth, rbFont);
      if (!left.isEmpty()) {
        tableWidth += rbFont.width(left);
        rb = rb.mid(left.size());
        tr_rb.append("<td>")
             .append(left)
             .append("</td>");
        tr_rt.append("<td/>");
      }
    }
    if (tableWidth > 0 && width > 0 && tableWidth + cellWidth + cellSpace > width // reduce table here
        && (!tr_rb.isEmpty() || !tr_rt.isEmpty())) {
      reduce();
      tr_rb.clear();
      tr_rt.clear();
      tableWidth = 0;
    }
    tableWidth += cellWidth + cellSpace;
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
  });
  if (!tr_rb.isEmpty() || !tr_rt.isEmpty())
    reduce();
  return ret;
}

// EOF
