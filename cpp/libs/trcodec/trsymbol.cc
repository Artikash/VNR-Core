// trsymbol.cc
// 3/5/2015 jichi
#include "trcodec/trsymbol.h"
#include "trcodec/trdefine.h"
#include "cpputil/cppregex.h"
#include <stack>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <QDebug>

#define SK_NO_QT
#define DEBUG "trencode.cc"
#include "sakurakit/skdebug.h"

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
    }
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
  return boost::regex_replace(s, rx, L"{{$1\\([-0-9<>]+\\)}}");
}

// Example text to decode: 4<3<1>>,<2>
static std::wstring decode_symbol_stack(const wchar_t *str, const trsymbol::decode_fun_t &fun)
{
  std::stack<std::wstring> tokens;
  std::vector<std::wstring> args;
  while (wchar_t ch = *str++)
    switch (ch) {
    case '<': tokens.push(std::wstring()); break;
    case '>': // reducee
      while (!tokens.empty() && !tokens.top().empty()) {
        args.push_back(tokens.top());
        tokens.pop();;
      }
      if (!tokens.empty()) {
        int id = 0;
        std::wstring t = fun(id, args);
        if (!t.empty()) {
          tokens.push(t);
          if (!args.empty())
            args.clear();
          break;
        }
      }
      DOUT("failed to decode symbol stack");
      return std::wstring();
    default:
      if (::isdigit(ch) || ch == '-')
        if (int id = ::wcstol(str - 1, nullptr, 10)) {
          std::wstring t = fun(id, args);
          if (!t.empty()) {
            tokens.push(t);
            if (!args.empty())
              args.clear();
          }
        }
      DOUT("failed to decode symbol character");
      return std::wstring();
    }

  std::wstring ret;
  return ret;
}

std::wstring trsymbol::decode_symbol(const std::wstring &text, const decode_fun_t &fun)
{
  static const boost::wregex rx(L"{{" TR_RE_TOKEN L"(<[-0-9<>]+>)" L"}}");
  return boost::regex_replace(text, rx, [&fun](const boost::wsmatch &m) {
    const std::wstring &matched_text = m[1];
    return decode_symbol_stack(matched_text.c_str(), fun);
  });
}

// EOF
