// trdeccde.cc
// 3/5/2015 jichi

#include "trcodec/trdecode.h"
#include "trcodec/trdecoderule.h"
#include <boost/algorithm/string.hpp>
#include <boost/lambda/lambda.hpp>
//#include <QDebug>

//#define DEBUG_RULE // output the rule that is applied

/** Private class */

class TranslationDecoderPrivate
{
public:
  //QReadWriteLock lock;

  TranslationDecodeRule *rules; // use array for performance reason
  size_t ruleCount;

  TranslationDecoderPrivate() : rules(nullptr), ruleCount(0) {}
  ~TranslationDecoderPrivate() { if (rules) delete[] rules; }

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
    rules = new TranslationDecodeRule[size];
    ruleCount = size;
  }
};

/** Public class */

// Construction

TranslationDecoder::TranslationDecoder() : d_(new D) {}
TranslationDecoder::~TranslationDecoder() { delete d_; }

int TranslationDecoder::size() const { return d_->ruleCount; }
bool TranslationDecoder::isEmpty() const { return !d_->ruleCount; }

void TranslationDecoder::clear() { d_->clear(); }

// Initialization
void TranslationDecoder::setRules(const TranslationRuleList &rules)
{
  if (rules.empty())
    d_->clear();
  else {
    d_->reset(rules.size());
    size_t i = 0;
    for (auto p = rules.cbegin(); p != rules.cend(); ++p)
      d_->rules[i++].init(*p);
  }
}

// Translation
void TranslationDecoder::decode(std::wstring &result, int category, int limit) const
{
  if (!d_->ruleCount || !d_->rules)
    return;
  std::wstring lastResult;
  for (int count = 0; lastResult != result && (!limit || count < limit); count++) {
    //qDebug() << count;
    lastResult = result;
    for (size_t i = 0; i < d_->ruleCount; i++) {
      const auto &rule = d_->rules[i];
      if (rule.is_valid()
          && rule.match_category(category)
          && (!count || rule.is_symbolic()))
        rule.replace(result);
    }
  }
}

// EOF
