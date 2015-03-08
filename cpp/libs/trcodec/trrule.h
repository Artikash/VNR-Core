#ifndef TRRULE_H
#define TRRULE_H

// trrule.h
// 9/20/2014 jichi

#include "trcodec/trflag.h"
#include <list>
#include <string>

struct TranslationRule : TranslationFlagStruct
{
  std::wstring token,   // the LHS token
               source,  // the RHS source
               target;  // the RHS target

  // Features
  int id,
      category;

  bool match_category(int v) const { return !v || !category || v & category; }

  TranslationRule() : id(0), category(0) {}
};

typedef std::list<TranslationRule> TranslationRuleList;

#endif // TRRULE_H
