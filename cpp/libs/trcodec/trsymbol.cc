// trsymbol.cc
// 3/5/2015 jichi
#include "trcodec/trsymbol.h"
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
//#include <QDebug>

bool trsymbol::contains_symbol(const std::wstring &s)
{ return boost::algorithm::contains(s, L"[[") && boost::algorithm::contains(s, L"]]"); }

std::wstring trsymbol::create_symbol(const std::wstring &token, int id)
{ return L"{{" + token + L"<" + std::to_wstring(id) + L">}}"; }

std::wstring trsymbol::encode_symbol(const std::wstring &s)
{
  static const boost::wregex rx(L"\\[\\[" L"([a-z]+)" L"\\]\\]");
  return boost::regex_replace(s, rx, L"{{\\1[0-9,<>]+}}");
}

// EOF
