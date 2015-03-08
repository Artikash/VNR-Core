#ifndef TRENCODERULE_H
#define TRENCODERULE_H

// trencoderule.h
// 9/20/2014 jichi

#include "trcodec/trrule.h"
#include "sakurakit/skglobal.h"
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <cstdint>

#ifdef __clang__
# pragma clang diagnostic ignored "-Wlogical-op-parentheses"
#endif // __clang__

class TranslationEncodeRule : private TranslationRule
{
  SK_EXTEND_CLASS(TranslationEncodeRule, TranslationRule)

  std::wstring token;
  int category;

  mutable bool valid; // whether the object is valid
  mutable std::wstring target;
  mutable std::wstring source;
  mutable boost::wregex *source_re; // cached compiled regex

public:
  using Base::is_symbolic;

  TranslationEncodeRule()
    : valid(false)
    , source_re(nullptr)
  {}

  ~TranslationEncodeRule()
  { if (source_re) delete source_re; }

  bool match_category(int v) const { return !v || !category || v & category; }

  void init(const TranslationRule &param);
  bool is_valid() const { return valid; }

  // Replacement
private:
  void init_source(); // may throw regular expression exception
  void cache_target() const;

  void string_replace(std::wstring &ret) const;
  void regex_replace(std::wstring &ret) const;

  bool string_exists(const std::wstring &t) const // inline to make this function faster
  { return is_icase() ? boost::algorithm::icontains(t, source) : boost::algorithm::contains(t, source); }

  bool regex_exists(const std::wstring &t) const;

  bool exists(const std::wstring &text) const
  { return is_regex() ? regex_exists(text) : string_exists(text); }

public:
  bool replace(std::wstring &ret) const
  {
    if (exists(ret)) {
      if (is_regex())
        regex_replace(ret);
      else
        string_replace(ret);
      return true;
    }
    return false;
  }
};

#endif // TRENCODERULE_H
