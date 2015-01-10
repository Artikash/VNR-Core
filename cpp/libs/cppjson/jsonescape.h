#ifndef JSONESCAPE_H
#define JSONESCAPE_H

// jsonescape.h
// 10/12/2014 jichi

#include "cppjson/cppjson.h"
#include <sstream>

CPPJSON_BEGIN_NAMESPACE

// See: http://stackoverflow.com/questions/7724448/simple-json-string-escape-for-c
// UNICODE chars are not handled, and that's why it is called escape_basic_string instead of escape.
template <typename _CharT, typename _Traits, typename _Alloc>
inline std::basic_string<_CharT, _Traits, _Alloc> escape_basic_string(const std::basic_string<_CharT, _Traits, _Alloc> &input)
{
  std::basic_ostringstream<_CharT, _Traits, _Alloc> is;
  for (auto it = input.cbegin(); it != input.cend(); ++it)
    switch (*it) {
    case '\\':  is << "\\\\"; break;
    case '"':   is << "\\\""; break;
    case '/':   is << "\\/"; break;
    case '\b':  is << "\\b"; break;
    case '\f':  is << "\\f"; break;
    case '\n':  is << "\\n"; break;
    case '\r':  is << "\\r"; break;
    case '\t':  is << "\\t"; break;
    default: is << *it;
    }
  return is.str();
}

CPPJSON_END_NAMESPACE

#endif // JSONESCAPE_H
