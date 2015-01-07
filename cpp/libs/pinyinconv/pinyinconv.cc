// pinyinconv.cc
// 1/7/2015 jichi

#include "pinyinconv/pinyinconv.h"
#include <boost/algorithm/string.hpp>
#include <cstdint> // for uint8_t
#include <fstream>
#include <sstream>
#include <unordered_map>
//#include <iostream>
#include <QDebug>

/** Helpers */

namespace { // unnamed

// CH_DELIM = '\t'
enum : char { CH_COMMENT = '#' }; // beginning of a comment

// See: https://github.com/lxneng/xpinyin
// aoeiv:
// - aeiouü
// - āēīōūǖ
// - áéíóúǘ
// - ǎěǐǒǔǚ
// - àèìòùǜ
const wchar_t *TONE_MARKS[] = { // a, o, e, ui, i, iu, u
  L"aeiou\u00fc"
  , L"\u0101\u0113\u012b\u014d\u016b\u01d6"
  , L"\u00e1\u00e9\u00ed\u00f3\u00fa\u01d8"
  , L"\u01ce\u011b\u01d0\u01d2\u01d4\u01da"
  , L"\u00e0\u00e8\u00ec\u00f2\u00f9\u01dc"
};

struct PinyinEntry
{
  std::string latin;
  uint8_t tone; // 0,1,2,3,4

  explicit PinyinEntry(const std::string &latin = std::string(), uint8_t tone = 0)
    : latin(latin), tone(tone) {}

  wchar_t tone_char() const
  {
    if (!latin.empty())
      switch (latin[latin.size() - 1]) {
      case 'a': return TONE_MARKS[tone][0];
      case 'e': return TONE_MARKS[tone][1];
      case 'i': return TONE_MARKS[tone][2];
      case 'o': return TONE_MARKS[tone][3];
      case 'u': return TONE_MARKS[tone][4];
      }
    return 0;
  }
};

} // unnamed namespace

/** Private class */

class PinyinConverterPrivate
{
public:
  typedef std::unordered_map<wchar_t, PinyinEntry> map_type;
  map_type map;
};

/** Public class */

// Construction

PinyinConverter::PinyinConverter() : d_(new D) {}
PinyinConverter::~PinyinConverter() { delete d_; }

int PinyinConverter::size() const { return d_->map.size(); }
bool PinyinConverter::isEmpty() const { return d_->map.empty(); }

void PinyinConverter::clear() { d_->map.clear(); }

// Initialization
bool PinyinConverter::addFile(const std::wstring &path)
{
#ifdef _MSC_VER
  std::ifstream fin(path);
#else
  std::string spath(path.begin(), path.end());
  std::ifstream fin(spath.c_str());
#endif // _MSC_VER
  if(!fin.is_open())
    return false;
  //fin.imbue(UTF8_LOCALE);

  std::string line;
  while (std::getline(fin, line))
    if (line.size() >= 3 && line[0] != CH_COMMENT) {
      std::string pinyin;
      int order;
      std::istringstream(line)
          >> std::hex >> order >> pinyin;
      if (order <= 0xffff && !pinyin.empty()) {
        int tone = (pinyin[pinyin.size() - 1] - '0') % 5;
        pinyin.pop_back();
        boost::to_lower(pinyin); // always use lower case
        d_->map[order] = PinyinEntry(pinyin, tone);
      }
      //qDebug() << order << QString::fromStdString(py);
    }

  fin.close();
  return true;
}

// Conversion

std::wstring PinyinConverter::convert(const std::wstring &text, const std::wstring &delim, bool tone) const
{
  if (text.empty() || d_->map.empty())
    return text;

  std::wstring ret;
  for (size_t i = 0; i < text.size(); i++) {
    wchar_t ch = text[i];
    if (!::isascii(ch)) {
      auto p = d_->map.find(ch);
      if (p != d_->map.end()) {
        const auto &py = p->second;
        ret.append(py.latin.begin(), py.latin.end());
        if (tone)
          if (wchar_t tc = py.tone_char())
            ret[ret.size() - 1] = tc;
        if (!delim.empty())
          ret.append(delim);
        continue;
      }
    }
    ret.push_back(ch);
  }
  return ret;
}

// EOF
