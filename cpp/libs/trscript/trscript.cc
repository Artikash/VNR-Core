// trscript.cc
// 9/20/2014 jichi

#include "trscript/trscript.h"
#include "trscript/trrule.h"
#include "cpputil/cpplocale.h"
#include <fstream>
//#include <QDebug>

#define SK_NO_QT
#define DEBUG "trscript.cc"
#include "sakurakit/skdebug.h"

//#define DEBUG_RULE // output the rule that is applied

#define SCRIPT_CH_COMMENT   L'#'    // indicate the beginning of a line comment
#define SCRIPT_CH_DELIM     L'\t'   // deliminator of the rule pair
#define SCRIPT_CH_REGEX     L'r'    // a regex rule
#define SCRIPT_CH_ICASE     L'i'    // case insensitive
#define SCRIPT_CH_NAME      L'n'    // a name without suffix
#define SCRIPT_CH_SUFFIX    L's'    // a name with suffix
#define SCRIPT_CH_FORCE     L'f'    // a name with suffix

/** Helpers */

namespace { // unnamed

const std::locale UTF8_LOCALE = ::cpp_utf8_locale<wchar_t>();

} // unnamed namespace

/** Private class */

class TranslationScriptManagerPrivate
{

public:
  //QReadWriteLock lock;

  TranslationScriptRule *rules; // use array for performance reason
  size_t ruleCount;

  TranslationScriptManagerPrivate() : rules(nullptr), ruleCount(0) {}
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

//bool TranslationScriptManager::isLinkEnabled() const { return d_->link; }
//void TranslationScriptManager::setLinkEnabled(bool t) { d_->link = t; }

//std::wstring TranslationScriptManager::linkStyle() const { return d_->linkStyle; }
//void TranslationScriptManager::setLinkStyle(const std::wstring &css) { d_->linkStyle = css; }

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
  if (!fin.is_open()) {
    DOUT("unable to open file");
    return false;
  }
  fin.imbue(UTF8_LOCALE);

  TranslationScriptRule::param_type param;
  TranslationScriptRule::param_list params; // id, pattern, text, regex
  size_t parentCount = 0;
  for (std::wstring line; std::getline(fin, line);)
    if (!line.empty() && line[0] != SCRIPT_CH_COMMENT) {
      param.clear_flags();
      size_t pos = 0;
      for (; pos < line.size() && line[pos] != SCRIPT_CH_DELIM; pos++)
        switch (line[pos]) {
        case SCRIPT_CH_FORCE: param.f_force = true; break;
        case SCRIPT_CH_REGEX: param.f_regex = true; break;
        case SCRIPT_CH_ICASE: param.f_icase = true; break;
        case SCRIPT_CH_NAME: param.f_parent = true; break;
        case SCRIPT_CH_SUFFIX: param.f_child = true; break;
        }
      if (pos == line.size())
        continue;
      //line.pop_back(); // remove trailing '\n'
      const wchar_t *cur = line.c_str() + pos;
      long id = ::wcstol(cur, const_cast<wchar_t **>(&cur), 10); // base 10
      if (cur && *cur++) { // skip first delim
        param.category = ::wcstol(cur, const_cast<wchar_t **>(&cur), 10); // base 10
        if (cur && *cur++) {
          param.id = id ? std::to_wstring((long long)id) : std::wstring();
          if (const wchar_t *delim = ::wcschr(cur, SCRIPT_CH_DELIM)) {
            param.source.assign(cur, delim - cur);
            param.target.assign(delim + 1);
          } else {
            param.source.assign(cur);
            param.target.clear();
          }
          if (!param.source.empty()) {
            if (!param.f_child)
              parentCount++;
            params.push_back(param);
            //qDebug() << QString::fromStdWString(param.id) << param.category << QString::fromStdWString(param.source) << QString::fromStdWString(param.target);
          }
        }
      }
    }

  fin.close();

  if (params.empty()) {
    d_->clear();
    return false;
  }

  //QWriteLocker locker(&d_->lock);
  d_->reset(parentCount);

  size_t i = 0;
  for (auto p = params.cbegin(), q = params.cend(); p != params.cend(); ++p)
    if (p->f_child) {
      if (q == params.cend())
        q = p;
    } else if (p->f_parent && q != params.cend()) {
      d_->rules[i++].init_list(*p, q, p);
      q = params.cend();
    } else
      d_->rules[i++].init(*p);

  return true;
}

// Translation
std::wstring TranslationScriptManager::translate(const std::wstring &text, int category, bool mark) const
{
  //QReadLocker locker(&d_->lock);
  std::wstring ret = text;
#ifdef DEBUG_RULE
  std::wstring previous = text;
#endif // DEBUG_RULE
  if (d_->ruleCount && d_->rules)
    for (size_t i = 0; i < d_->ruleCount; i++) {
      const auto &rule = d_->rules[i];
      //qDebug() << QString::fromStdWString(rule.id) << rule.flags << QString::fromStdWString(rule.source) << QString::fromStdWString(rule.target);
      if (rule.is_valid() && rule.match_category(category))
        rule.replace(ret, mark);

#ifdef DEBUG_RULE
      if (previous != ret)
        DOUT(QString::fromStdWString(rule.source) << QString::fromStdWString(rule.target) << QString::fromStdWString(ret));
      previous = ret;
#endif // DEBUG_RULE
    }
  return ret;
}

// EOF
