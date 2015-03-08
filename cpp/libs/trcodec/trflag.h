#ifndef TRFLAG_H
#define TRFLAG_H

// trflag.h
// 9/20/2014 jichi

#include <cstdint>

enum TranslationFlag : uint8_t {
  TranslationRegexFlag = 1          // source is regular expression
  , TranslationIcaseFlag = 1 << 1   // ignore case for source
};

struct TranslationFlagStruct
{
  mutable uint8_t flags;
  TranslationFlagStruct() : flags(0) {}

  bool has_flag(uint8_t f) const { return flags & f; }
  void set_flag(uint8_t f, bool t) { if (t) flags |= f; else flags &= ~f; }

#define DEF_FLAG(_method, _flag) \
  bool is_##_method() const { return has_flag(_flag); } \
  void set_##_method(bool t) { set_flag(_flag, t); }

  DEF_FLAG(regex, TranslationRegexFlag)
  DEF_FLAG(icase, TranslationIcaseFlag)
#undef DEF_FLAG
};

#endif // TRFLAG_H
