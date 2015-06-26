// main.cc
// 6/25/2015 jichi
//
// Measure text width in Qt
// http://doc.qt.io/qt-4.8/qfontmetrics.html
// http://stackoverflow.com/questions/1337523/measuring-text-width-in-qt
#include <QtGui>
#include <functional>

#define RUBY_OPEN   '{'
#define RUBY_SEP    '|'
#define RUBY_CLOSE  '}'

bool containsRuby(const QString &text)
{ return text.contains(RUBY_OPEN) && text.contains(RUBY_CLOSE); }

typedef std::function<bool (const QString &rb, const QString &rt)> ruby_fun_t; // return if stop iteration
void iterRuby(const QString &text,  const ruby_fun_t &fun)
{
  QString rb, rt, plainText;
  bool rubyOpenFound = false,
       rubySepFound = false;
  foreach (const QChar &ch, text) {
    switch (ch.unicode()) {
    case RUBY_OPEN:
      if (!plainText.isEmpty()) {
        if (!fun(plainText, rt))
          return;
        plainText.clear();
      }
      rubyOpenFound = true;
      break;
    case RUBY_SEP:
      if (rubyOpenFound)
        rubySepFound = true;
      break;
    case RUBY_CLOSE:
      if (rubyOpenFound) {
        if ((!rb.isEmpty() || !rt.isEmpty()) && !fun(rb, rt))
           return;
        rubySepFound = rubyOpenFound = false;
        rb.clear();
        rt.clear();
      }
      break;
    default:
      (!rubyOpenFound ? plainText : rubySepFound ? rt : rb).push_back(ch);
    }
  }
  if (!plainText.isEmpty())
    rb = plainText;
  if (!rb.isEmpty() || !rt.isEmpty())
    fun(rb, rt);
}

QString renderRuby(const QString &text, int width, const QFont &rbFont, const QFont &rtFont, int cellSpan = 1)
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
  iterRuby(text, fun);
  if (!tr_rb.isEmpty() || !tr_rt.isEmpty())
    ret.append("<table>")
       .append("<tr class='rb'>").append(tr_rt).append("</tr>")
       .append("<tr class='rt'>").append(tr_rb).append("</tr>")
       .append("</table>");
  return ret;
}

QString removeRuby(const QString &text)
{ return text; }

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  auto w = new QTextEdit;

  QString h = "{hello|ruby} what {hello|ruby}  again {hello|ruby}  another {hello|ruby} very long long text";
  w->resize(200, 300);
  int contentWidth = w->contentsRect().width();
  QFont rbFont = w->font(),
        rtFont = w->font();
  rbFont.setBold(true);
  rtFont.setUnderline(true);
  h = renderRuby(h, contentWidth, rbFont, rtFont);
  h.prepend(
    "<style type='text/css'>"
    ".rb { text-decoration: underline; }"
    "</style>"
  );
  qDebug() << h;
  w->setHtml(h);
  w->show();
  return a.exec();
}
