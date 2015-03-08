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

  std::wstring token,
               target,
               source;
  mutable bool valid; // whether the object is valid

public:
  TranslationDecodeRule() : valid(false) {}
  ~TranslationDecodeRule() {}

  bool match_category(int v) const { return !v || !category || v & category; }

  void init(const TranslationRule &param);
  bool is_valid() const { return valid; }

public:
  /// Render target replacement
  std::wstring render(const std::vector<std::wstring> &args, bool mark = true) const;
};

#endif // TRDECODERULE_H
