#ifndef TRRULE_H
#define TRRULE_H

// trrule.h
// 9/20/2014 jichi

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <list>
#include <cstdint>

#ifdef __clang__
# pragma clang diagnostic ignored "-Wlogical-op-parentheses"
#endif // __clang__

struct TranslationScriptParam
{
  typedef TranslationScriptParam Self;

  std::wstring id,
               source,
               target;
  uint8_t f_regex,  // this is a regex
          f_parent, // this is a name
          f_child;  // this is a name

  //TranslationScriptParam() {} // uninitialized

  void clear_flags() { f_regex = f_parent = f_child = 0; }
};

struct TranslationScriptRule
{
  typedef TranslationScriptRule Self;

  enum Flag : uint8_t {
    RegexFlag =  1
    , ListFlag = 1 << 1
  };

  uint8_t flags;
  std::wstring id,
               source,
               target;
  Self *children;
  size_t child_count;
  mutable bool valid; // whether the object is valid
  mutable boost::wregex *source_re; // cached compiled regex

public:
  typedef TranslationScriptParam param_type;
  typedef std::list<param_type> param_list;

  TranslationScriptRule()
    : flags(0)
    , children(nullptr)
    , child_count(0)
    , valid(false)
    , source_re(nullptr)
  {}

  ~TranslationScriptRule()
  {
    if (children) delete[] children;
    if (source_re) delete source_re;
  }

  bool is_valid() const { return valid; }

  void init(const param_type &param, bool precompile_regex = true);
  void init_list(const param_type &param,
                 param_list::const_iterator begin,
                 param_list::const_iterator end);

  // Replacement
private:
  bool is_regex() const { return flags & RegexFlag; }
  bool is_list() const { return flags & ListFlag; }

  std::wstring render_target() const;

  void string_replace(std::wstring &ret, bool link) const;
  void regex_replace(std::wstring &ret, bool link) const;

  bool string_exists(const std::wstring &t) const { return boost::algorithm::contains(t, source); }
  bool regex_exists(const std::wstring &t) const;
  bool children_replace(std::wstring &ret, bool link) const;

public:
  bool replace(std::wstring &ret, bool link) const
  {
    if (!(is_regex() ? regex_exists(ret) : string_exists(ret)))
      return false;
    if (!is_list() || !children_replace(ret, link)) {
      if (is_regex())
        regex_replace(ret, link);
      else
        string_replace(ret, link);
    }
    return true;
  }
};

#endif // TRRULE_H
