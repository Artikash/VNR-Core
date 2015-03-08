// trsymbol.cc
// 3/5/2015 jichi
#include "trcodec/trsymbol.h"
#include <boost/algorithm/string.hpp>

bool trsymbol::contains_symbol(const std::wstring &s)
{ return boost::algorithm::contains(s, L"[[") && boost::algorithm::contains(s, L"]]"); }

std::wstring trsymbol::render_symbol(const std::wstring &token, int id)
{ return L"{{" + token + L"(" + std::to_wstring(id) + L")}}"; }

// EOF
