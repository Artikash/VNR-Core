#ifndef TRRULE_H
#define TRRULE_H

// trrule.h
// 9/20/2014 jichi

#include <list>
#include <string>

class TranslationRule
{
public:
  std::wstring token,   // the LHS token
               source,  // the RHS source
               target;  // the RHS target

  // Features
  int id,
      category;

  // Flags
  bool f_regex,  // this is a regex
       f_icase;  // case insensitive
};

typedef std::list<TranslationRule> TranslationRuleList;

#endif // TRRULE_H
