#ifndef TRRULE_H
#define TRRULE_H

// trrule.h
// 9/20/2014 jichi

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <cstdint>
#include <list>

struct TranslationScriptParam
{
  typedef TranslationScriptParam Self;

  std::wstring id,
               source,
               target;
  uint8_t f_regex,
          f_name,
          f_suffix;

  std::list<Self> children;

  TranslationScriptParam() {} // uninitialized
  TranslationScriptParam(const std::wstring &id, const std::wstring &source, const std::wstring target,
                         bool regex, bool name, bool suffix)
    : id(id), source(source), target(target)
    , f_regex(regex), f_name(name), f_suffix(suffix) {}

  void clear_flags() { f_regex = f_name = f_suffix = 0; }
};

struct TranslationScriptRule
{
  typedef TranslationScriptRule Self;

  enum Flag : uint8_t {
    RegexFlag =     1
    , NameFlag =    1 << 1
    , SuffixFlag =  1 << 2
  };

  uint8_t flags;
  mutable bool valid;
  std::wstring id,
               source,
               target;
  boost::wregex *source_re; // cached compiled regex

  Self *children;
  size_t child_count;

public:
  TranslationScriptRule()
    : flags(0)
    , valid(false)
    , source_re(nullptr)
    , children(nullptr)
    , child_count(0)
  {}

  ~TranslationScriptRule()
  {
    if (children) delete children;
    if (source_re) delete source_re;
  }

  bool is_valid() const { return valid; }

  void init(const TranslationScriptParam &Param);

  // Replacement
private:
  bool is_regex() const { return flags & RegexFlag; }

  std::wstring render_target() const;

  void string_replace(std::wstring &ret, bool link) const;
  void regex_replace(std::wstring &ret, bool link) const;

public:
  void replace(std::wstring &ret, bool link) const
  {
    if (is_regex())
      regex_replace(ret, link);
    else if (boost::algorithm::contains(ret, source)) // check exist first which is faster and could avoid rendering target
      string_replace(ret, link);
  }
};

#endif // TRRULE_H
