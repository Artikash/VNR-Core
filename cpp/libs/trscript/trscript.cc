// trscript.cc
// 9/20/2014 jichi

#include "trscript/trscript.h"

#include <list> // instead of QList which is slow that stores pointers instead of elements
#include <fstream>

//#include <tuple>
#include <boost/tuple/tuple.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>

#include "cpputil/cpplocale.h"

#define SK_NO_QT
#define DEBUG "trscript.cc"
#include "sakurakit/skdebug.h"

//#define DEBUG_RULE // output the rule that is applied

#define SCRIPT_CH_COMMENT   '#' // indicate the beginning of a line comment
#define SCRIPT_CH_REGEX     'r'

#define SCRIPT_RULE_DELIM   '\t' // deliminator of the rule pair
enum { SCRIPT_RULE_DELIM_LEN = 1 };
//enum { SCRIPT_RULE_DELIM_LEN = (sizeof(SCRIPT_RULE_DELIM)  - 1) }; // strlen

/** Helpers */

namespace { // unnamed

const std::locale UTF8_LOCALE = ::cpp_utf8_locale<wchar_t>();

// Force inlining text rendering functions
#define _underline_text(text) \
  (L"<span style=\"text-decoration:underline\">" + (text) + L"</span>")

#define render_text(text, underline) \
  ((text).empty() ? std::wstring(): (underline) ? _underline_text((text)) : (text))

struct TranslationScriptRule
{
  mutable bool empty;
  std::wstring source;
  std::wstring target;
  boost::wregex *sourceRe; // cached compiled regex

  TranslationScriptRule() : empty(true), sourceRe(nullptr) {}
  ~TranslationScriptRule() { if (sourceRe) delete sourceRe; }

  void init(const std::wstring &s, const std::wstring &t, bool regex)
  {
    empty = false;
    if (regex) {
      boost::wregex *re = nullptr;
      try {
        re = new boost::wregex(s);
      } catch (...) { // boost::bad_pattern
        DWOUT("invalid regex pattern:" << s);
        empty = true;
        return;
      }

      sourceRe = re;
      target = t;
      //target.replace('$', '\\'); // convert Javascript RegExp to Perl
    } else {
      source = s;
      target = t;
    }
  }

  // Replacement
private:
  void string_replace(std::wstring &ret, bool underline) const
  {
    if (target.empty())
      boost::erase_all(ret, source);
    else
      boost::replace_all(ret, source, render_text(target, underline));
  }

  void regex_replace(std::wstring &ret, bool underline) const
  {
    //Q_ASSERT(sourceRe);
    try  {
      boost::wsmatch m; // search first, which has less opportunity to happen
      // match_default is the default value
      // format_all is needed to enable all features, but it is sligntly slower
      if (boost::regex_search(ret, m, *sourceRe))
        ret = boost::regex_replace(ret, *sourceRe, render_text(target, underline),
            boost::match_default|boost::format_all);
    } catch (...) {
      DWOUT("invalid regex expression:" << target);
      empty = true;
    }
  }

public:
  void replace(std::wstring &ret, bool underline) const
  {
    if (sourceRe)
      regex_replace(ret, underline);
    else if (boost::algorithm::contains(ret, source)) // check exist first which is faster and could avoid rendering target
      string_replace(ret, underline);
  }
};

} // unnamed namespace

/** Private class */

class TranslationScriptManagerPrivate
{

public:
  //QReadWriteLock lock;

  TranslationScriptRule *rules; // use array for performance reason
  int ruleCount;

  bool underline;

  TranslationScriptManagerPrivate() : rules(nullptr), ruleCount(0), underline(false) {}
  ~TranslationScriptManagerPrivate() { if (rules) delete[] rules; }

  void clear()
  {
    ruleCount = 0;
    if (rules) {
      delete[] rules;
      rules = nullptr;
    }
  }

  void reset(int size)
  {
    //DOUT(size);
    //Q_ASSERT(size > 0);
    ruleCount = size;
    if (rules)
      delete[] rules;
    rules = new TranslationScriptRule[size];

  }
};

/** Public class */

// Construction

TranslationScriptManager::TranslationScriptManager() : d_(new D) {}
TranslationScriptManager::~TranslationScriptManager() { delete d_; }

int TranslationScriptManager::size() const { return d_->ruleCount; }
bool TranslationScriptManager::isEmpty() const { return !d_->ruleCount; }

bool TranslationScriptManager::isUnderline() const { return d_->underline; }
void TranslationScriptManager::setUnderline(bool value) { d_->underline = value; }

void TranslationScriptManager::clear() { d_->clear(); }

// Initialization
bool TranslationScriptManager::loadFile(const std::wstring &path)
{
  std::wifstream fin(path);
  if(!fin.is_open()) {
    DOUT("Unable to open file");
    return false;
  }
  fin.imbue(UTF8_LOCALE);

  std::list<boost::tuple<std::wstring, std::wstring, bool> > lines; // pattern, text, regex

  for (std::wstring line; std::getline(fin, line);)
    if (!line.empty()) {
      bool regex = false;
      int textStartIndex = 1; // index of the text after flags, +1 to skip \t
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
        lines.push_back(boost::make_tuple(left, right, regex));
    }

  fin.close();

  if (lines.empty()) {
    d_->clear();
    return false;
  }

  //QWriteLocker locker(&d_->lock);
  d_->reset(lines.size());

  int i = 0;
  BOOST_FOREACH (const auto &it, lines)
    d_->rules[i++].init(boost::get<0>(it), boost::get<1>(it), boost::get<2>(it));

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
    for (int i = 0; i < d_->ruleCount; i++) {
      const auto &rule = d_->rules[i];
      if (!rule.empty)
        rule.replace(ret, d_->underline);

#ifdef DEBUG_RULE
      if (previous != ret) {
        if (rule.sourceRe)
          DOUT(QString::fromStdWString(rule.target) << QString::fromStdWString(ret));
        else
          DOUT(QString::fromStdWString(rule.source) << QString::fromStdWString(rule.target) << QString::fromStdWString(ret));
      }
      previous = ret;
#endif // DEBUG_RULE
    }
  return ret;
}

// EOF
