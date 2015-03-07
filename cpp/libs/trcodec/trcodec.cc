// trcodec.cc
// 3/5/2015 jichi

#include "trcodec/trcodec.h"
#include "trcodec/trdefine.h"
#include "trcodec/trencode.h"
#include "trcodec/trrule.h"
#include "cpputil/cpplocale.h"
#include <boost/algorithm/string.hpp>
#include <boost/lambda/lambda.hpp>
#include <fstream>
#include <vector>
//#include <QDebug>

#define SK_NO_QT
#define DEBUG "trcodec.cc"
#include "sakurakit/skdebug.h"

//#define DEBUG_RULE // output the rule that is applied

/** Helpers */

namespace { // unnamed

const std::locale UTF8_LOCALE = ::cpp_utf8_locale<wchar_t>();

} // unnamed namespace

/** Private class */

class TranslationCodecPrivate
{

public:
  //QReadWriteLock lock;

  TranslationEncoder *encoder;
  TranslationEncoder *decoder;

  TranslationCodecPrivate() : encoder(new TranslationEncoder), decoder(new TranslationEncoder) {}
  ~TranslationCodecPrivate() {  delete encoder; delete decoder; }

  void clear()
  {
    encoder->clear();
    decoder->clear();
  }

  static bool loadRules(const std::wstring &path, TranslationRuleList &rules);

};

bool TranslationCodecPrivate::loadRules(const std::wstring &path, TranslationRuleList &rules)
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

  const auto column_sep = boost::lambda::_1 == TRSCRIPT_CH_COLUMNSEP,
             feature_sep = boost::lambda::_1 == TRSCRIPT_CH_FEATURESEP;
  std::vector<std::wstring> cols;
  std::vector<std::string> features;
  for (std::wstring line; std::getline(fin, line);)
    if (!line.empty() && line[0] != TRSCRIPT_CH_COMMENT) {
      boost::split(cols, line, column_sep);
      if (!cols.empty()) {
        TranslationRule rule = {};
        if (cols.size() > TRSCRIPT_COLUMN_TOKEN)
          rule.token = cols[TRSCRIPT_COLUMN_TOKEN];
        if (cols.size() > TRSCRIPT_COLUMN_TOKEN)
          rule.source = cols[TRSCRIPT_COLUMN_SOURCE];
        if (cols.size() > TRSCRIPT_COLUMN_TARGET)
          rule.target = cols[TRSCRIPT_COLUMN_TARGET];
        if (cols.size() > TRSCRIPT_COLUMN_FEATURE) {
          std::string feature(cols[TRSCRIPT_COLUMN_FEATURE].begin(),
                              cols[TRSCRIPT_COLUMN_FEATURE].end());
          if (!feature.empty()) {
            boost::split(features, feature, feature_sep);
            if (!features.empty()) {
              if (features.size() > TRSCRIPT_FEATURE_ID)
                rule.id = std::stoi(features[TRSCRIPT_FEATURE_ID].c_str());
              if (features.size() > TRSCRIPT_FEATURE_CATEGORY)
                rule.category = std::stoi(features[TRSCRIPT_FEATURE_CATEGORY].c_str());
              if (features.size() > TRSCRIPT_FEATURE_FLAGS) {
                const std::string &flags = features[TRSCRIPT_FEATURE_FLAGS];
                for (size_t pos = 0; pos < flags.size(); pos++)
                  switch (flags[pos]) {
                  case TRSCRIPT_CH_REGEX: rule.f_regex = true; break;
                  case TRSCRIPT_CH_ICASE: rule.f_icase = true; break;
                  }
              }
            }
          }
        }

        if (!rule.token.empty() && !rule.source.empty())
          rules.push_back(rule);
      }
    }

  fin.close();
  return true;
}

/** Public class */

// Construction

TranslationCodec::TranslationCodec() : d_(new D) {}
TranslationCodec::~TranslationCodec() { delete d_; }

int TranslationCodec::size() const { return d_->encoder->size(); }
bool TranslationCodec::isEmpty() const { return d_->encoder->isEmpty(); }

//bool TranslationCodec::isLinkEnabled() const { return d_->link; }
//void TranslationCodec::setLinkEnabled(bool t) { d_->link = t; }

//std::wstring TranslationCodec::linkStyle() const { return d_->linkStyle; }
//void TranslationCodec::setLinkStyle(const std::wstring &css) { d_->linkStyle = css; }

void TranslationCodec::clear() { d_->clear(); }

// Initialization
bool TranslationCodec::loadScript(const std::wstring &path)
{
  TranslationRuleList rules; // id, pattern, text, regex
  D::loadRules(path, rules);

  if (rules.empty()) {
    d_->clear();
    return false;
  }

  //QWriteLocker locker(&d_->lock);

  d_->encoder->setRules(rules);
  d_->decoder->setRules(rules);

  return true;
}

// Translation
std::wstring TranslationCodec::encode(const std::wstring &text, int category, int limit) const
{
  if (text.empty() || d_->encoder->isEmpty())
    return text;
  std::wstring ret = text;
  d_->encoder->encode(ret, category, limit);
  return ret;
}

// EOF
