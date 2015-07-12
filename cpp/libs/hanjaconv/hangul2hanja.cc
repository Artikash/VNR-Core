// hangul2hanja.cc
// 1/6/2015 jichi

#include "hanjaconv/hangul2hanja.h"
#include "hanjaconv/hangul2hanja_p.h"
#include "hanjaconv/hangulconfig.h"
#include "cpputil/cpplocale.h"
#include <boost/foreach.hpp>
#include <fstream>
#include <list>
#include <utility> // for pair
//#include <iostream>
//#include <QDebug>

/** Helpers for parsing files */

namespace { // unnamed

enum : char {
  CH_COMMENT = '#'  // beginning of a comment
  , CH_DELIM = '\t' // deliminator
};

const std::locale UTF8_LOCALE = ::cpp_utf8_locale<wchar_t>();

} // unnamed namespace

/** Public class */

// Construction

HangulHanjaConverter::HangulHanjaConverter() : d_(new D) {}
HangulHanjaConverter::~HangulHanjaConverter() { delete d_; }

int HangulHanjaConverter::size() const { return d_->entry_count; }
bool HangulHanjaConverter::isEmpty() const { return !d_->entry_count; }

void HangulHanjaConverter::clear() { d_->clear(); }

// Initialization
bool HangulHanjaConverter::loadFile(const std::wstring &path)
{
#ifdef _MSC_VER
  std::wifstream fin(path);
#else
  std::string spath(path.begin(), path.end());
  std::wifstream fin(spath.c_str());
#endif // _MSC_VER
  if (!fin.is_open())
    return false;
  fin.imbue(UTF8_LOCALE);

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

std::wstring HangulHanjaConverter::convert(const std::wstring &text) const
{
  if (text.size() < HANJA_MIN_SIZE || !d_->entries) // at least two elements
    return text;

  std::wstring ret = text;
  d_->replace(ret);
  return ret;
}

void HangulHanjaConverter::collect(const std::wstring &text, const collect_fun_t &fun) const
{
  if (text.empty() || !d_->entries) // at least two elements
    return;
  if (text.size() < HANJA_MIN_SIZE)
    fun(0, text.size(), nullptr);
  d_->collect(text, fun);
}

// EOF
