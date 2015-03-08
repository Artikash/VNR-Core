// trsymbol.cc
// 3/5/2015 jichi
#include "trcodec/trsymbol.h"
#include "trcodec/trdefine.h"
#include "cpputil/cppregex.h"
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
//#include <QDebug>

bool trsymbol::contains_raw_symbol(const std::wstring &s)
{ return boost::algorithm::contains(s, L"[[") && boost::algorithm::contains(s, L"]]"); }

bool trsymbol::contains_encoded_symbol(const std::wstring &s)
{ return boost::algorithm::contains(s, L"{{") && boost::algorithm::contains(s, L">}}"); }

std::wstring trsymbol::create_symbol_target(const std::wstring &token, int id, int argc)
{
  std::wstring ret = L"{{";
  ret += token;
  ret += L'<';
  ret += std::to_wstring(id);
  if (argc) {
    for (int i = 1; i <= argc; i++) {
      ret += L'$';
      if (i < 10)
        ret += (L'0' + i);
      else {
        ret += L'{';
        ret += std::to_wstring(i);
        ret += L'}';
      }
      ret += L',';
    }
    ret.pop_back(); // remove the last ','
  }
  ret += L">}}";
  return ret;
}

int trsymbol::count_raw_symbols(const std::wstring &s)
{
  if (!contains_raw_symbol(s))
    return 0;
  static const boost::wregex rx(L"\\[\\[" TR_RE_TOKEN L"\\]\\]");
  return ::cpp_wregex_count(s, rx);
}

std::wstring trsymbol::encode_symbol(const std::wstring &s)
{
  static const boost::wregex rx(L"\\[\\[" L"(" TR_RE_TOKEN L")" L"\\]\\]");
  return boost::regex_replace(s, rx, L"{{$1\\([0-9,<>]+\\)}}");
}

// EOF
