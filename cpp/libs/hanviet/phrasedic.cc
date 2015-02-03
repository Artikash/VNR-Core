// phrasedic.cc
// 2/2/2015 jichi

#include "hanviet/phrasedic.h"
#include "cpputil/cpplocale.h"
#include "unistr/unichar.h"
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <fstream>
#include <list>
#include <utility>
//#include <iostream>
//#include <QDebug>

/** Helpers */

namespace { // unnamed

enum : char {
  CH_COMMENT = L'#'  // beginning of a comment
  , CH_DELIM = L'='  // deliminator
};

const std::locale UTF8_LOCALE = ::cpp_utf8_locale<wchar_t>();

struct HanVietPhraseEntry
{
  std::wstring han,     // source
               viet;    // target

  void reset(const std::wstring &first, const std::wstring &second)
  {
    han = first;
    viet = second;
  }
};

} // unnamed namespace

/** Private class */

class HanVietPhraseDictionaryPrivate
{
public:
  typedef HanVietPhraseEntry entry_type;

  entry_type *entries;
  size_t entry_count;

  HanVietPhraseDictionaryPrivate() : entries(nullptr), entry_count(0) {}
  ~HanVietPhraseDictionaryPrivate() { if (entries) delete[] entries; }

  void clear()
  {
    entry_count = 0;
    if (entries) {
      delete[] entries;
      entries = nullptr;
    }
  }

  void resize(size_t size)
  {
    //Q_ASSERT(size > 0);
    if (entry_count != size) {
      clear(); // clear first for thread-safety
      if (entries)
        delete[] entries;
      entries = new entry_type[size];
      entry_count = size;
    }
  }
};

/** Public class */

// Construction

HanVietPhraseDictionary::HanVietPhraseDictionary() : d_(new D) {}
HanVietPhraseDictionary::~HanVietPhraseDictionary() { delete d_; }

int HanVietPhraseDictionary::size() const { return d_->entry_count; }
bool HanVietPhraseDictionary::isEmpty() const { return !d_->entry_count; }

void HanVietPhraseDictionary::clear() { d_->clear(); }

// Initialization
bool HanVietPhraseDictionary::addFile(const std::wstring &path)
{
#ifdef _MSC_VER
  std::wifstream fin(path);
#else
  std::string spath(path.begin(), path.end());
  std::wifstream fin(spath.c_str());
#endif // _MSC_VER
  if(!fin.is_open())
    return false;
  fin.imbue(UTF8_LOCALE);

  std::list<std::pair<std::wstring, std::wstring> > lines; // han, viet

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

std::wstring HanVietPhraseDictionary::lookup(const std::wstring &text) const
{
  if (text.empty() || !d_->entries) // at least two elements
    return std::wstring();
  for (size_t i = 0; i < d_->entry_count; i++) {
    const auto &e = d_->entries[i];
    if (e.han == text)
      return e.viet;
  }
  return std::wstring();
}

std::wstring HanVietPhraseDictionary::translate(const std::wstring &text, bool mark) const
{
  if (text.empty() || !d_->entries) // at least two elements
    return text;

  std::wstring ret = text;

  for (size_t i = 0; i < d_->entry_count; i++) {
    const auto &e = d_->entries[i];
    if (boost::contains(ret, e.han))
      boost::replace_all(ret, e.han, e.viet); // TODO: Render viet
  }

  return ret;
}

// EOF
