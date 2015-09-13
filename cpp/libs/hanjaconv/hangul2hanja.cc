// hangul2hanja.cc
// 1/6/2015 jichi

#include "hanjaconv/hangul2hanja.h"
#include "hanjaconv/hangul2hanja_p.h"
#include "hanjaconv/hanjadef_p.h"
#include <boost/foreach.hpp>
#include <fstream>
#include <utility> // for pair
//#include <iostream>
//#include <QDebug>

/** Public class */

// Construction

HangulHanjaConverter::HangulHanjaConverter() : d_(new D) {}
HangulHanjaConverter::~HangulHanjaConverter() { delete d_; }

int HangulHanjaConverter::size() const { return d_->entry_count; }
bool HangulHanjaConverter::isEmpty() const { return !d_->entry_count; }

void HangulHanjaConverter::clear() { d_->clear(); }

// Initialization
bool HangulHanjaConverter::loadFile(const wchar_t *path)
{
#ifdef _MSC_VER
  std::wifstream fin(path);
#else
  std::string spath(path, path + ::wcslen(path));
  std::wifstream fin(spath.c_str());
#endif // _MSC_VER
  if (!fin.is_open())
    return false;
  fin.imbue(HANJA_UTF8_LOCALE);

  std::list<std::pair<std::wstring, std::wstring> > lines; // hanja, hangul

  for (std::wstring line; std::getline(fin, line);)
    if (line.size() >= 3 && line[0] != CH_COMMENT) {
      size_t pos = line.find(CH_DELIM);
      if (pos != std::string::npos && 1 <= pos && pos < line.size() - 1)
        lines.push_back(std::make_pair(
            line.substr(0, pos),
            line.substr(pos + 1)));
    }

  fin.close();

  if (lines.empty()) {
    d_->clear();
    return false;
  }

  //QWriteLocker locker(&d_->lock);
  d_->resize(lines.size());

  size_t i = 0;
  BOOST_FOREACH (const auto &it, lines)
    d_->entries[i++].reset(it.first, it.second);

  return true;
}

// Conversion

std::wstring HangulHanjaConverter::convert(const wchar_t *text) const
{
  if (::wcslen(text) < HANJA_MIN_SIZE || !d_->entries) // at least two elements
    return text;

  std::wstring ret = text;
  d_->replace(ret);
  return ret;
}

void HangulHanjaConverter::collect(const wchar_t *text, size_t size, const collect_fun_t &fun) const
{
  if (!text[0] || !d_->entries) // at least two elements
    return;
  //if (!size)
  //  size = ::wcslen(text);
  if (size < HANJA_MIN_SIZE)
    fun(0, size, nullptr);
  else
    d_->collect(text, size, fun);
}

#ifdef WITH_LIB_UNISTR
#include "unistr/uniiter.h"

#ifdef WITHOUT_CXX_LAMBDA // when lambda is not available on, say, GCC 4.4

namespace { // unnamed
typedef std::list<std::pair<std::wstring, std::wstring> > RubyList;

class RubyCallback // for self::collect
{
  RubyList &ret;
  const wchar_t *word;
public:
  RubyCallback(RubyList &ret, const wchar_t *word)
    : ret(ret), word(word) {}
  void operator()(size_t start, size_t length, const wchar_t *hanja)
  {
    ret.push_back(std::make_pair(
      std::wstring(word + start, length)
      , hanja ? std::wstring(hanja) : std::wstring()
    ));
  }
};

class WordCallback // for uniiter::iter_words
{
  RubyList &ret;
  const wchar_t *sentence;
  const HangulHanjaConverter *self;
public:
  WordCallback(RubyList &ret, const wchar_t *sentence, const HangulHanjaConverter *self)
    : ret(ret), sentence(sentence), self(self) {}
  void operator()(size_t start, size_t length, bool isword)
  {
    //std::wstring word = sentence.substr(start, length);
    const wchar_t *word = sentence + start;
    if (!isword || length < HANJA_MIN_SIZE) // for better performance
      ret.push_back(std::make_pair(std::wstring(word, length), std::wstring()));
    else {
      RubyCallback fn(ret, word);
      self->collect(word, length, fn);
    }
  }
};
} // unnamed namespace

std::list<std::pair<std::wstring, std::wstring> >
HangulHanjaConverter::parseToList(const std::wstring &text) const
{
  std::list<std::pair<std::wstring, std::wstring> > ret;
  if (text.empty() || !d_->entries) // at least two elements
    return ret;
  const wchar_t *sentence = text.c_str();
  WordCallback fn(ret, sentence, this);
  uniiter::iter_words(sentence, text.size(), fn);
  return ret;
}

#else // when lambda is supported

std::list<std::pair<std::wstring, std::wstring> >
HangulHanjaConverter::parseToList(const std::wstring &text) const
{
  std::list<std::pair<std::wstring, std::wstring> > ret;
  if (text.empty() || !d_->entries) // at least two elements
    return ret;
  const wchar_t *sentence = text.c_str();
  uniiter::iter_words(sentence, text.size(), [&ret, sentence, this](size_t start, size_t length, bool isword) {
    //std::wstring word = sentence.substr(start, length);
    const wchar_t *word = sentence + start;
    if (!isword || length < HANJA_MIN_SIZE) // for better performance
      ret.push_back(std::make_pair(std::wstring(word, length), std::wstring()));
    else
      this->collect(word, length, [&ret, word](size_t start, size_t length, const wchar_t *hanja) {
        ret.push_back(std::make_pair(
          std::wstring(word + start, length)
          , hanja ? std::wstring(hanja) : std::wstring()
        ));
      });
  });
  return ret;
}

#endif // WITHOUT_CXX_LAMBDA

#endif // WITH_LIB_UNISTR

// EOF
