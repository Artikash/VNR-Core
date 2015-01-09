// trscript.cc
// 9/20/2014 jichi

#include "trscript/trscript.h"
#include "cpputil/cpplocale.h"
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <cstdint>
#include <list> // instead of QList which is slow that stores pointers instead of elements
#include <tuple>
#include <fstream>

#define SK_NO_QT
#define DEBUG "trscript.cc"
#include "sakurakit/skdebug.h"

//#define DEBUG_RULE // output the rule that is applied

#define SCRIPT_CACHE_REGEX // enable caching regex

#define SCRIPT_CH_COMMENT   '#' // indicate the beginning of a line comment
#define SCRIPT_CH_REGEX     'r'

#define SCRIPT_RULE_DELIM   '\t' // deliminator of the rule pair
enum { SCRIPT_RULE_DELIM_LEN = 1 };
//enum { SCRIPT_RULE_DELIM_LEN = (sizeof(SCRIPT_RULE_DELIM)  - 1) }; // strlen

/** Helpers */

namespace { // unnamed

const std::locale UTF8_LOCALE = ::cpp_utf8_locale<wchar_t>();

// Force inlining text rendering functions to avoid copying std::wstring on return
#define _underline_text(text) \
  (L"<span style=\"text-decoration:underline\">" + (text) + L"</span>")

#define render_text(text, underline) \
  ((text).empty() ? std::wstring(): (underline) ? _underline_text((text)) : (text))

class TranslationScriptRule
{
  enum Flag : uint8_t { RegexFlag = 1 };

  uint8_t flags;
  mutable bool valid;

#ifdef SCRIPT_CACHE_REGEX
  boost::wregex *sourceRe; // cached compiled regex
#endif // SCRIPT_CACHE_REGEX;

public:
  std::wstring source,
               target;

  TranslationScriptRule()
    : flags(0), valid(false)
#ifdef SCRIPT_CACHE_REGEX
    , sourceRe(nullptr)
#endif // SCRIPT_CACHE_REGEX
  {}

#ifdef SCRIPT_CACHE_REGEX
  ~TranslationScriptRule()
  {
    if (sourceRe) delete sourceRe;
  }
#endif // SCRIPT_CACHE_REGEX

  bool isValid() const { return valid; }

  void init(const std::wstring &s, const std::wstring &t, bool regex)
  {
    if (regex) {
      flags |= RegexFlag;
      try {
#ifdef SCRIPT_CACHE_REGEX
        sourceRe = new boost::wregex(s);
#else
        boost::wregex(s);
#endif // SCRIPT_CACHE_REGEX
      } catch (...) { // boost::bad_pattern
        DWOUT("invalid regex pattern:" << s);
        valid = false;
      }
    }

#ifdef SCRIPT_CACHE_REGEX
    if (!regex)
#endif // SCRIPT_CACHE_REGEX
      source = s;

    target = t;

    valid = true;
  }

  // Replacement
private:
  void string_replace(std::wstring &ret, bool link) const
  {
    if (target.empty())
      boost::erase_all(ret, source);
    else
      boost::replace_all(ret, source, render_text(target, link));
  }

  void regex_replace(std::wstring &ret, bool link) const
  {
    //Q_ASSERT(sourceRe);
    try  {
      boost::wsmatch m; // search first, which has less opportunity to happen
      // match_default is the default value
      // format_all is needed to enable all features, but it is sligntly slower
#ifdef SCRIPT_CACHE_REGEX
      const auto &re = *sourceRe;
#else
      boost::wregex re(source);
#endif // SCRIPT_CACHE_REGEX
      if (boost::regex_search(ret, m, re))
        ret = boost::regex_replace(ret, re, render_text(target, link),
            boost::match_default|boost::format_all);
    } catch (...) {
      DWOUT("invalid regex expression:" << target);
      valid = false;
    }
  }

public:
  void replace(std::wstring &ret, bool link) const
  {
    if (flags & RegexFlag)
      regex_replace(ret, link);
    else if (boost::algorithm::contains(ret, source)) // check exist first which is faster and could avoid rendering target
      string_replace(ret, link);
  }
};

} // unnamed namespace

/** Private class */

class TranslationScriptManagerPrivate
{

public:
  //QReadWriteLock lock;

  TranslationScriptRule *rules; // use array for performance reason
  size_t ruleCount;

  bool link;

  TranslationScriptManagerPrivate() : rules(nullptr), ruleCount(0), link(false) {}
  ~TranslationScriptManagerPrivate() { if (rules) delete[] rules; }

  void clear()
  {
    ruleCount = 0;
    if (rules) {
      delete[] rules;
      rules = nullptr;
    }
  }

  void reset(size_t size)
  {
    //DOUT(size);
    //Q_ASSERT(size > 0);
    //if (ruleCount != size) {
    ruleCount = size;
    if (rules)
      delete[] rules;
    rules = new TranslationScriptRule[size];
    //}
  }
};

/** Public class */

// Construction

TranslationScriptManager::TranslationScriptManager() : d_(new D) {}
TranslationScriptManager::~TranslationScriptManager() { delete d_; }

int TranslationScriptManager::size() const { return d_->ruleCount; }
bool TranslationScriptManager::isEmpty() const { return !d_->ruleCount; }

bool TranslationScriptManager::isLinkEnabled() const { return d_->link; }
void TranslationScriptManager::setLinkEnabled(bool t) { d_->link = t; }

void TranslationScriptManager::clear() { d_->clear(); }

// Initialization
bool TranslationScriptManager::loadFile(const std::wstring &path)
{
#ifdef _MSC_VER
  std::wifstream fin(path);
#else
  std::string spath(path.begin(), path.end());
  std::wifstream fin(spath.c_str());
#endif // _MSC_VER
  if(!fin.is_open()) {
    DOUT("unable to open file");
    return false;
  }
  fin.imbue(UTF8_LOCALE);

  std::list<std::tuple<std::wstring, std::wstring, bool> > lines; // pattern, text, regex

  for (std::wstring line; std::getline(fin, line);)
    if (!line.empty()) {
      bool regex = false;
      size_t textStartIndex = 1; // index of the text after flags, +1 to skip \t
      switch (line[0]) {
      case SCRIPT_CH_COMMENT: continue;
      case SCRIPT_CH_REGEX: regex = true; textStartIndex++; break;
      }
      std::wstring left, right;
      auto index = line.find(SCRIPT_RULE_DELIM, textStartIndex);
      if (index == std::wstring::npos)
        left = line.substr(textStartIndex);
      else {
        left = line.substr(textStartIndex, index - textStartIndex); //.trimmed()
        right = line.substr(index + SCRIPT_RULE_DELIM_LEN);
      }
      if (!left.empty())
        lines.push_back(std::make_tuple(left, right, regex));
    }

  fin.close();

  if (lines.empty()) {
    d_->clear();
    return false;
  }

  //QWriteLocker locker(&d_->lock);
  d_->reset(lines.size());

  size_t i = 0;
  BOOST_FOREACH (const auto &it, lines)
    d_->rules[i++].init(std::get<0>(it), std::get<1>(it), std::get<2>(it));

  return true;
}

// Translation
std::wstring TranslationScriptManager::translate(const std::wstring &text) const
{
  //QReadLocker locker(&d_->lock);
  std::wstring ret = text;
#ifdef DEBUG_RULE
  std::wstring previous = text;
#endif // DEBUG_RULE
  if (d_->ruleCount && d_->rules)
    for (size_t i = 0; i < d_->ruleCount; i++) {
      const auto &rule = d_->rules[i];
      if (rule.isValid())
        rule.replace(ret, d_->link);

#ifdef DEBUG_RULE
      if (previous != ret)
        DOUT(QString::fromStdWString(rule.source) << QString::fromStdWString(rule.target) << QString::fromStdWString(ret));
      previous = ret;
#endif // DEBUG_RULE
    }
  return ret;
}

// EOF
