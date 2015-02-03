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
  std::wstring id,
               source,
               target;
  int category;
  uint8_t f_regex,  // this is a regex
          f_icase,  // case insensitive
          f_parent, // this is a name
          f_child;  // this is a name+suffix

  //TranslationScriptParam() {} // uninitialized

  void clear_flags() { f_regex = f_parent = f_child = f_icase = 0; }
};

class TranslationScriptRule
{
  typedef TranslationScriptRule Self;

  enum Flag : uint8_t {
    ListFlag = 1
    , RegexFlag = 1 << 1
    , IcaseFlag = 1 << 2
  };

  uint8_t flags;
  std::wstring id,
               source,
               target;
  int category;
  Self *children;
  size_t child_count;
  mutable bool valid; // whether the object is valid
  mutable boost::wregex *source_re; // cached compiled regex

public:
  typedef TranslationScriptParam param_type;
  typedef std::list<param_type> param_list;

  TranslationScriptRule()
    : flags(0)
    , category(0)
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

  bool match_category(int v) const { return !v || !category || v == category; }

  void init(const param_type &param, bool precompile_regex = true);
  void init_list(const param_type &param,
                 param_list::const_iterator begin,
                 param_list::const_iterator end);

  // Replacement
private:
  bool is_list() const { return flags & ListFlag; }
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

  void string_replace(std::wstring &ret, bool mark) const;
  void regex_replace(std::wstring &ret, bool mark) const;

  bool string_exists(const std::wstring &t) const // inline to make this function faster
  { return is_icase() ? boost::algorithm::icontains(t, source) : boost::algorithm::contains(t, source); }

  bool regex_exists(const std::wstring &t) const;
  bool children_replace(std::wstring &ret, bool mark) const;

public:
  bool exists(const std::wstring &text) const
  { return is_regex() ? regex_exists(text) : string_exists(text); }

  bool replace(std::wstring &ret, bool mark) const
  {
    if (!exists(ret))
      return false;
    if (!is_list() || !children_replace(ret, mark)) {
      if (is_regex())
        regex_replace(ret, mark);
      else
        string_replace(ret, mark);
    }
    return true;
  }
};

#endif // TRRULE_H
