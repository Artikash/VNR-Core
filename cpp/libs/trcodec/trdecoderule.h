#ifndef TRDDECODERULE_H
#define TRDDECODERULE_H

// trdecoderule.h
// 9/20/2014 jichi

#include "trcodec/trrule.h"
#include "sakurakit/skglobal.h"
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <vector>

class TranslationDecodeRule : private TranslationRule
{
  SK_EXTEND_CLASS(TranslationDecodeRule, TranslationRule)

  mutable bool valid; // whether the object is valid

public:
  using Base::match_category;

  TranslationDecodeRule() : valid(false) {}
  ~TranslationDecodeRule() {}

  void init(const TranslationRule &param);
  bool is_valid() const { return valid; }

public:
  /// Render target replacement
  std::wstring render(const std::vector<std::wstring> &args, bool mark = true) const;
};

#endif // TRDECODERULE_H
