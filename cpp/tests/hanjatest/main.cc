// main.cc
// 1/6/2014 jichi
#include "hanjaconv/hanjaconv.h"
#include "hanjaconv/hanjachars.h"
#include <QtCore>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <locale>
#include "cpputil/cpplocale.h"

void hangul_iter_parse(const std::wstring &text, const std::function<void (size_t start, size_t length)> &fun)
{
  size_t pos = 0; // beginning of a word
  for (size_t i =  0; i < text.size(); i++) {
    //wchar_t ch = text[i];
    //if (::isspace(ch) || ::ispunct(ch)) // this need std::locale(ja_JP.UTF-8)
    QChar ch = text[i];
    if (ch.isSpace() || ch.isPunct()) {
      if (pos < i)
        fun(pos, i - pos);
      fun(i, 1);
      pos = i + 1;
    }
  }
  if (pos < text.size())
    fun(pos, text.size() - pos);
}

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
  bool addFile(const QString &path);

  // Parse

  // Replace the characters according to the script, thread-safe
  QString convert(const QString &text) const;

  // Replace the characters according to the script, thread-safe
  QList<QList<QPair<QString, QString> > > parse(const QString &text) const;
};

class HangulHanjaConverter_p
{
public:
  HanjaConverter conv;
  HangulHanjaConverter_p() {}
};

/** Public class */

// - Construction -

HangulHanjaConverter::HangulHanjaConverter() : d_(new D) {}
HangulHanjaConverter::~HangulHanjaConverter() { delete d_; }

int HangulHanjaConverter::size() const { return d_->conv.size(); }
bool HangulHanjaConverter::isEmpty() const { return d_->conv.isEmpty(); }

void HangulHanjaConverter::clear() { d_->conv.clear(); }

bool HangulHanjaConverter::addFile(const QString &path)
{ return d_->conv.addFile(path.toStdWString()); }

QString HangulHanjaConverter::convert(const QString &text) const
{ return QString::fromStdWString(d_->conv.convert(text.toStdWString())); }

QList<QList<QPair<QString, QString> > >
HangulHanjaConverter::parse(const QString &text) const
{
  typedef QPair<QString, QString> Pair;
  typedef QList<Pair> PairList;
  typedef QList<PairList> PairListList;

  PairListList ret;
  std::wstring sentence = text.toStdWString();
  ::hangul_iter_parse(sentence, [&ret, &sentence, this](size_t start, size_t stop) {
    std::wstring word = sentence.substr(start, stop);
    ret.append(PairList());
    auto &l = ret.last();

    this->d_->conv.collect(word, [&l, &word](size_t start, size_t length, const wchar_t *hanja) {
      l.append(qMakePair(
        QString::fromWCharArray(word.c_str() + start, length)
        , hanja ? QString::fromWCharArray(hanja) : QString()
      ));
    });
  });
  return ret;
}

const std::locale UTF8_LOCALE = ::cpp_utf8_locale<wchar_t>();
std::locale LOCALE("ja_JP.UTF-8");
//std::locale LOCALE(UTF8_LOCALE);
void iterwords(const std::wstring &text, const std::function<void (size_t start, size_t length)> &fun)
{
  //bool be = false; // is end
  //size_t pos = 0; // beginning of a word
  //for (size_t i =  0; i < text.size(); i++) {
  //  wchar_t ch = text[i];
  //  QChar qch(ch);
  //  //if (::isspace(ch) || ::ispunct(ch))
  //  //if (std::isspace(ch, LOCALE) || std::ispunct(ch, LOCALE))
  //  if (qch.isSpace() || qch.isPunct()) {
  //    be = true;
  //    if (pos < i)
  //      fun(pos, i - pos);
  //    fun(i, 1);
  //    pos = i + 1;
  //  } else
  //    be = false;
  //}
  //if (pos < text.size())
  //  fun(pos, text.size() - pos);
  size_t pos = 0; // beginning of a word
  for (size_t i =  0; i < text.size(); i++) {
    //wchar_t ch = text[i];
    //if (::isspace(ch) || ::ispunct(ch)) // this need std::locale(ja_JP.UTF-8)
    QChar ch = text[i];
    if (ch.isSpace() || ch.isPunct()) {
      if (pos < i)
        fun(pos, i - pos);
      fun(i, 1);
      pos = i + 1;
    }
  }
  if (pos < text.size())
    fun(pos, text.size() - pos);
}

int main()
{
  QString path = "/Users/jichi/opt/stream/Library/Dictionaries/hanja/dic6.txt";
  HangulHanjaConverter conv;
  conv.addFile(path);
  //std::wstring ws = L"【오쿠라 리소나】「미나톤이";
  std::wstring ws = L"test";
  QString qs = QString::fromStdWString(ws);

  QString t = conv.convert(qs);
  qDebug() << t;

  auto l = conv.parse(qs);
  foreach (auto it, l)
    foreach (auto q, it)
      qDebug() << "output:"<<q.first;

  return 0;
}

// EOF

#if 0
int main()
{
  qDebug() << "enter";

  const wchar_t *path = L"/Users/jichi/opt/stream/Library/Dictionaries/hanja/dic6.txt";
  //std::wstring s = L"공주";
  //std::wstring s = L"상냥한 곳은 바뀌지 않았어. 옛날은 좀 더 형분적인 표현(이었)였지만.";
  //std::wstring s = L"지금 무엇을 하고 있을까. 자신의 판단으로 대답해도 좋은 것인가 헤매었을 것이다.";

  // Sentence from http://ko.wikipedia.org/wiki/자유_콘텐츠
  // Ambiguity: 이용이
  // Supposed to be 利用이 instead of 이容易
  //std::wstring s = L"자유 콘텐츠는 저작권이 소멸된 퍼블릭 도메인은 물론, 저작권이 있지만 위 기준에 따라 자유롭게 이용이 허락된 콘텐츠도 포함한다.";
  //std::wstring s = L"容易이용이";
  //std::wstring s = L"【오쿠라 리소나】「미나톤이 우리 집을 나온 후의 이야기군요. 외동딸(이었)였으므로, 그렇다면 집에서 인귀로라고 해도 좋다고 하는 것이 된 것 같아」";
  std::wstring s = L"【오쿠라 리소나】「미나톤이";

  //std::wstring s = L"test";
  //wchar_t ch = L'【';
  //wchar_t ch = L'「';
  //qDebug() << ::ispunct(ch) << QChar(ch).isPunct();

  HanjaConverter conv;
  bool ok = conv.addFile(path);
  qDebug() << ok;

  std::wstring t = conv.convert(s);
  qDebug() << QString::fromStdWString(t);

  std::list <std::list<std::pair<std::wstring, std::wstring> > > ret;
  iterwords(s, [&ret, &s, &conv](size_t start, size_t stop) {

    std::wstring text = s.substr(start, stop);
    qDebug() << "----:" << start << ":"<<QString::fromStdWString(text);

    ret.push_back(std::list<std::pair<std::wstring, std::wstring> >());
    auto &l = ret.back();

    conv.collect(text, [&l, &text](size_t start, size_t length, const wchar_t *hanja) {
      l.push_back(std::make_pair(
        text.substr(start, length)
        , hanja ? std::wstring(hanja) : std::wstring()
      ));
    });
  });

  qDebug() << ret.size();
  for (auto it = ret.begin(); it != ret.end(); ++it)
    for (auto p = it->begin(); p != it->end(); ++p)
      qDebug() << QString::fromStdWString(p->first) << ":" << QString::fromStdWString(p->second);

  qDebug() << "leave";
  return 0;
}
#endif // 0
