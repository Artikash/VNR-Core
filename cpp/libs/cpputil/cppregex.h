#ifndef CPPREGEX_H
#define CPPREGEX_H

#include <boost/regex.hpp>

// This function might raise
template<_String, _Regex>
inline bool cpp_regex_contains(const _String &str, _Regex pattern)
{ return boost::regex_search(str, pattern); }

// This function might raise
// http://stackoverflow.com/questions/8283735/count-number-of-matches
template<_String, _Regex>
inline std::ptrdiff_t cpp_regex_count(const _String &str, _Regex pattern)
{
  return std::distance(
      boost::sregex_iterator(str.begin(), str.end(), pattern),
      boost::sregex_iterator());
}

#endif // CPPUNICODE_H
