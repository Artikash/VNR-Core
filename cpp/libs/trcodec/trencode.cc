// trenccde.cc
// 3/5/2015 jichi

#include "trcodec/trencode.h"
#include "trcodec/trdefine.h"
#include "trcodec/trencoderule.h"
#include "cpputil/cpplocale.h"
#include <boost/algorithm/string.hpp>
#include <boost/lambda/lambda.hpp>
#include <fstream>
#include <list>
#include <vector>
#include <QDebug>

#define SK_NO_QT
#define DEBUG "trscript.cc"
#include "sakurakit/skdebug.h"

//#define DEBUG_RULE // output the rule that is applied

/** Helpers */

namespace { // unnamed

const std::locale UTF8_LOCALE = ::cpp_utf8_locale<wchar_t>();

} // unnamed namespace

/** Private class */

class TranslationEncoderPrivate
{

public:
  //QReadWriteLock lock;

  TranslationEncodeRule *rules; // use array for performance reason
  size_t ruleCount;

  TranslationEncoderPrivate() : rules(nullptr), ruleCount(0) {}
  ~TranslationEncoderPrivate() { if (rules) delete[] rules; }

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
    rules = new TranslationEncodeRule[size];
    ruleCount = size;
  }
};

/** Public class */

// Construction

TranslationEncoder::TranslationEncoder() : d_(new D) {}
TranslationEncoder::~TranslationEncoder() { delete d_; }

int TranslationEncoder::size() const { return d_->ruleCount; }
bool TranslationEncoder::isEmpty() const { return !d_->ruleCount; }

//bool TranslationEncoder::isLinkEnabled() const { return d_->link; }
//void TranslationEncoder::setLinkEnabled(bool t) { d_->link = t; }

//std::wstring TranslationEncoder::linkStyle() const { return d_->linkStyle; }
//void TranslationEncoder::setLinkStyle(const std::wstring &css) { d_->linkStyle = css; }

void TranslationEncoder::clear() { d_->clear(); }

// Initialization
bool TranslationEncoder::loadScript(const std::wstring &path)
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

  std::list<TranslationEncodeRule::param_type> params; // id, pattern, text, regex
  const auto column_sep = boost::lambda::_1 == TRSCRIPT_CH_COLUMNSEP,
             feature_sep = boost::lambda::_1 == TRSCRIPT_CH_FEATURESEP;
  std::vector<std::wstring> cols;
  std::vector<std::string> features;
  for (std::wstring line; std::getline(fin, line);)
    if (!line.empty() && line[0] != TRSCRIPT_CH_COMMENT) {
      boost::split(cols, line, column_sep);
      if (!cols.empty()) {
        TranslationEncodeRule::param_type param = {};
        if (cols.size() > TRSCRIPT_COLUMN_TOKEN)
          param.token = cols[TRSCRIPT_COLUMN_TOKEN];
        if (cols.size() > TRSCRIPT_COLUMN_TOKEN)
          param.source = cols[TRSCRIPT_COLUMN_SOURCE];
        if (cols.size() > TRSCRIPT_COLUMN_TARGET)
          param.target = cols[TRSCRIPT_COLUMN_TARGET];
        if (cols.size() > TRSCRIPT_COLUMN_FEATURE) {
          std::string feature(cols[TRSCRIPT_COLUMN_FEATURE].begin(),
                              cols[TRSCRIPT_COLUMN_FEATURE].end());
          if (!feature.empty()) {
            boost::split(features, feature, feature_sep);
            if (!features.empty()) {
              if (features.size() > TRSCRIPT_FEATURE_ID)
                param.id = std::stoi(features[TRSCRIPT_FEATURE_ID].c_str());
              if (features.size() > TRSCRIPT_FEATURE_CATEGORY)
                param.category = std::stoi(features[TRSCRIPT_FEATURE_CATEGORY].c_str());
              if (features.size() > TRSCRIPT_FEATURE_FLAGS) {
                const std::string &flags = features[TRSCRIPT_FEATURE_FLAGS];
                for (size_t pos = 0; pos < flags.size(); pos++)
                  switch (flags[pos]) {
                  case TRSCRIPT_CH_REGEX: param.f_regex = true; break;
                  case TRSCRIPT_CH_ICASE: param.f_icase = true; break;
                  }
              }
            }
          }
        }

        if (!param.token.empty() && !param.source.empty())
          params.push_back(param);
      }
    }

  fin.close();

  if (params.empty()) {
    d_->clear();
    return false;
  }

  //QWriteLocker locker(&d_->lock);
  d_->reset(params.size());

  size_t i = 0;
  for (auto p = params.cbegin(); p != params.cend(); ++p)
    d_->rules[i++].init(*p);

  return true;
}

// Translation
std::wstring TranslationEncoder::encode(const std::wstring &text, int category) const
{
  //QReadLocker locker(&d_->lock);
  std::wstring ret = text;
  if (d_->ruleCount && d_->rules)
    for (size_t i = 0; i < d_->ruleCount; i++) {
      const auto &rule = d_->rules[i];
      //qDebug() << QString::fromStdWString(rule.id) << rule.flags << QString::fromStdWString(rule.source) << QString::fromStdWString(rule.target);
      if (rule.is_valid() && rule.match_category(category))
        rule.replace(ret);
    }
  return ret;
}

// EOF
