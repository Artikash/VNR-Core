#ifndef TRENCODERULE_H
#define TRENCODERULE_H

// trencoderule.h
// 9/20/2014 jichi

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <cstdint>

#ifdef __clang__
# pragma clang diagnostic ignored "-Wlogical-op-parentheses"
#endif // __clang__

struct TranslationEncodeParam
{
  std::wstring id,
               source,
               target;
  int category;
  uint8_t f_force,  // force apply certain transformation
          f_regex,  // this is a regex
          f_icase,  // case insensitive
          f_parent, // this is a name
          f_child;  // this is a name+suffix

  //TranslationEncodeParam() {} // uninitialized

  void clear_flags() { f_force = f_regex = f_parent = f_child = f_icase = 0; }
};

class TranslationEncodeRule
{
  typedef TranslationEncodeRule Self;

  enum Flag : uint8_t {
    ForceFlag = 1
    , RegexFlag = 1 << 1
    , IcaseFlag = 1 << 2
  };

  uint8_t flags;
  std::wstring id,
               source,
               target;
  int category;
  mutable bool valid; // whether the object is valid
  mutable boost::wregex *source_re; // cached compiled regex

public:
  typedef TranslationEncodeParam param_type;

  TranslationEncodeRule()
    : flags(0)
    , category(0)
    , valid(false)
    , source_re(nullptr)
  {}

  ~TranslationEncodeRule()
  { if (source_re) delete source_re; }

  bool is_valid() const { return valid; }

  bool match_category(int v) const { return !v || !category || v & category; }

  void init(const param_type &param, bool precompile_regex = true);

  // Replacement
private:
  bool is_force() const { return flags & ForceFlag; }
  bool is_regex() const { return flags & RegexFlag; }
  bool is_icase() const { return flags & IcaseFlag; }

  void cache_re() const // may throw
  {
    if (!source_re) {
      if (is_icase())
        source_re = new boost::wregex(source, boost::wregex::icase);
      else
        source_re = new boost::wregex(source);
    }
  }

  std::wstring render_target() const;

  void string_replace(std::wstring &ret) const;
  void regex_replace(std::wstring &ret) const;

  bool string_exists(const std::wstring &t) const // inline to make this function faster
  { return is_icase() ? boost::algorithm::icontains(t, source) : boost::algorithm::contains(t, source); }

  bool regex_exists(const std::wstring &t) const;

  bool exists(const std::wstring &text) const
  { return is_force() || (is_regex() ? regex_exists(text) : string_exists(text)); }

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
