#ifndef UNISTR_H
#define UNISTR_H

// unistr.h
// 2/2/2015 jichi

#include "unistr/unichar.h"
#include <iterator> // for back_inserter

namespace unistr {

template <typename SequenceT>
inline void to_thin(SequenceT &s)
{ std::transform(s.cbegin(), s.cend(), s.begin(), wide2thin); }

template <typename SequenceT>
inline SequenceT to_thin_copy(const SequenceT &s)
{
  SequenceT t;
  std::transform(s.begin(), s.end(), std::back_inserter(t), wide2thin);
  return t;
}

template <typename SequenceT>
inline void to_wide(SequenceT &s)
{ std::transform(s.cbegin(), s.cend(), s.begin(), thin2wide); }

template <typename SequenceT>
inline SequenceT to_wide_copy(const SequenceT &s)
{
  SequenceT t;
  std::transform(s.begin(), s.end(), std::back_inserter(t), thin2wide);
  return t;
}


} // namespace unistr

#endif // UNISTR_H
