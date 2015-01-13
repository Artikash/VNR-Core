#ifndef TRRULE_H
#define TRRULE_H

// trrule.h
// 9/20/2014 jichi

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <cstdint>

struct TranslationScriptParam
{
  std::wstring id,
               source,
               target;
  bool regex;

  TranslationScriptParam() {} // uninitialized
  TranslationScriptParam(const std::wstring &id, const std::wstring &source,
                         const std::wstring target, bool regex)
    : id(id), source(source), target(target), regex(regex) {}
};

struct TranslationScriptRule
{
  enum Flag : uint8_t { RegexFlag = 1 };

  uint8_t flags;
  mutable bool valid;

  boost::wregex *sourceRe; // cached compiled regex

public:
  std::wstring id,
               source,
               target;

  TranslationScriptRule()
    : flags(0)
    , valid(false)
    , sourceRe(nullptr)
  {}

  ~TranslationScriptRule()
  { if (sourceRe) delete sourceRe; }

  bool isValid() const { return valid; }

  void init(const TranslationScriptParam &Param);

  // Replacement
private:
  bool isRegex() const { return flags & RegexFlag; }

  std::wstring render_target() const;

  void string_replace(std::wstring &ret, bool link) const;
  void regex_replace(std::wstring &ret, bool link) const;

public:
  void replace(std::wstring &ret, bool link) const
  {
    if (isRegex())
      regex_replace(ret, link);
    else if (boost::algorithm::contains(ret, source)) // check exist first which is faster and could avoid rendering target
      string_replace(ret, link);
  }
};

#endif // TRRULE_H
