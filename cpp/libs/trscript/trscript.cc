// trscript.cc
// 9/20/2014 jichi

#include "trscript/trscript.h"
#include "cppjson/jsonescape.h"
#include "cpputil/cpplocale.h"
//#include "cpputil/cppstring.h"
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <list>
#include <vector>

#define SK_NO_QT
#define DEBUG "trscript.cc"
#include "sakurakit/skdebug.h"

//#define DEBUG_RULE // output the rule that is applied

#define SCRIPT_CACHE_REGEX // enable caching compiled regex, 10 times faster but cost more memory

#define SCRIPT_CH_COMMENT   L'#' // indicate the beginning of a line comment
#define SCRIPT_CH_DELIM     L'\t' // deliminator of the rule pair
#define SCRIPT_CH_REGEX     L'r'

//enum { SCRIPT_RULE_DELIM_LEN = 1 };
//enum { SCRIPT_RULE_DELIM_LEN = (sizeof(SCRIPT_RULE_DELIM)  - 1) }; // strlen

/** Helpers */

namespace { // unnamed

const std::locale UTF8_LOCALE = ::cpp_utf8_locale<wchar_t>();

struct TranslationScriptRow
{
  std::wstring id,
               source,
               target;
  bool regex;

  TranslationScriptRow() {}
  TranslationScriptRow(const std::wstring &id, const std::wstring &source, const std::wstring target, bool regex)
    : id(id), source(source), target(target), regex(regex) {}
};

struct TranslationScriptRule
{
  enum Flag : uint8_t { RegexFlag = 1 };

  uint8_t flags;
  mutable bool valid;

#ifdef SCRIPT_CACHE_REGEX
  boost::wregex *sourceRe; // cached compiled regex
#endif // SCRIPT_CACHE_REGEX;

public:
  std::wstring id,
               source,
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

  void init(const TranslationScriptRow &row)
  {
    id = row.id;
//#ifdef SCRIPT_CACHE_REGEX
//    if (!row.regex)
//#endif // SCRIPT_CACHE_REGEX
    source = row.source;
    target = row.target;

    if (row.regex) {
      flags |= RegexFlag;
      try {
#ifdef SCRIPT_CACHE_REGEX
        sourceRe = new boost::wregex(row.source);
#else
        boost::wregex(s);
#endif // SCRIPT_CACHE_REGEX
      } catch (...) { // boost::bad_pattern
        DWOUT("invalid term: " << row.id << ", regex pattern: " << row.source);
        valid = false;
        return;
      }
    }
    valid = true; // must do this at the end
  }

  // Replacement
private:

  static std::string escape(const std::wstring &t)
  {
    std::string r = cpp_json::escape_basic_string(t, true); // true = escape all chars
    if (r.find('\'') != std::string::npos)
      boost::replace_all(r, "'", "\\'");
    return r;
  }

  //static bool requires_escape(const std::wstring &t)
  //{
  //  //static const std::wstring s = ::cpp_wstring_of(CPPJSON_ESCAPE_STRING).append(L"'");
  //  //return t.find_first_of(s) != std::wstring::npos;
  //  for(const wchar_t *it = t.c_str(); *it; ++it) {
  //    auto ch = *it;
  //    if (!isascii(ch) || ch == '\'' || cpp_json::escape_special_char(ch))
  //      return false;
  //  }
  //  return true;
  //}

  bool isRegex() const { return flags & RegexFlag; }

  // A sample expected output without escape:
  // <a href='json://{"type":"term","id":12345,"source":"pattern","target":"text"}'>pattern</a>
  std::wstring render_target() const
  {
    std::wstring ret = L"{\"type\":\"term\"";
    ret.append(L",\"id\":")
       .append(id);
    if (!isRegex()) { // do not save regex pattern to save memory
      if (!source.empty() && !::isdigit(source[0])) { // do not save escaped floating number
        std::string s = escape(source);
        ret.append(L",\"source\":\"")
           .append(s.cbegin(), s.cend())
           .push_back('"');
      }
      if (!target.empty()) {
        std::string s = escape(target);
        ret.append(L",\"target\":\"")
           .append(s.cbegin(), s.cend())
           .push_back('"');
      }
    }
    ret.push_back('}');

    ret.insert(0, L"<a href='json://");
    ret.push_back('\'');

    //if (!linkStyle.empty())
    //  ret.append(" style=\"")
    //     .append(linkStyle)
    //     .append("\"");

    ret.push_back('>');
    ret.append(target)
       .append(L"</a>");
    return ret;
  }

  void string_replace(std::wstring &ret, bool link) const
  {
    if (boost::algorithm::contains(ret, source)) { // do not render_target if no match
      if (target.empty())
        boost::erase_all(ret, source);
      else
        boost::replace_all(ret, source,
            target.empty() ? std::wstring() : !link ? target : render_target());
    }
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
        ret = boost::regex_replace(ret, re,
            target.empty() ? std::wstring() : !link ? target : render_target(),
            boost::match_default|boost::format_all);
    } catch (...) {
      DWOUT("invalid regex expression:" << target);
      valid = false;
    }
  }

public:
  void replace(std::wstring &ret, bool link) const
  {
    if (isRegex())
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
  std::wstring linkStyle;

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
    clear(); // clear first for thread-safety
    if (rules)
      delete[] rules;
    rules = new TranslationScriptRule[size];
    ruleCount = size;
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

std::wstring TranslationScriptManager::linkStyle() const { return d_->linkStyle; }
void TranslationScriptManager::setLinkStyle(const std::wstring &css) { d_->linkStyle = css; }

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

  TranslationScriptRow row;
  std::list<TranslationScriptRow> rows; // id, pattern, text, regex
  for (std::wstring line; std::getline(fin, line);)
    if (line.size()> 3) {
      size_t pos = 0;
      switch (line[0]) {
      case SCRIPT_CH_COMMENT: continue;
      case SCRIPT_CH_REGEX: row.regex = true; pos++; break;
      default: row.regex = false;
      }
      //line.pop_back(); // remove trailing '\n'
      wchar_t *cur;
      long id = ::wcstol(line.c_str() + pos, &cur, 10);
      if (id && *cur++) { // skip first delim
        row.id = std::to_wstring((long long)id);
        if (wchar_t *delim = ::wcschr(cur, SCRIPT_CH_DELIM)) {
          row.source.assign(cur, delim - cur);
          row.target.assign(delim + 1);
        } else {
          row.source.assign(cur);
          row.target.clear();
        }
        if (!row.source.empty())
          rows.push_back(row);
      }
      //qDebug() << row.id << QString::fromStdWString(row.source) << QString::fromStdWString(row.target);
    }

  fin.close();

  if (rows.empty()) {
    d_->clear();
    return false;
  }

  //QWriteLocker locker(&d_->lock);
  d_->reset(rows.size());

  size_t i = 0;
  BOOST_FOREACH (const auto &it, rows)
    d_->rules[i++].init(it);

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
      //qDebug() << rule.id << rule.flags << QString::fromStdWString(rule.source) << QString::fromStdWString(rule.target);
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
