// hangulconv.cc
// 1/6/2015
#include "pyhangul/hangulconv.h"
#include "pyhangul/hangulparse.h"
#include "hanjaconv/hanjaconv.h"

//#define DEBUG "HangulHanjaConverter.cc"
//#include "sakurakit/skdebug.h"

/** Private class */

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

// EOF
