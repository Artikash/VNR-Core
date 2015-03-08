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

// Precompiled regex
namespace { namespace rx {

const boost::wregex
  // To test [[x#123]]
  raw_symbol(
    L"\\[\\["
      TR_RE_TOKEN
      L"(:#[0-9]+)?"
    L"\\]\\]"
  )

  // To capture [[x#123]]'s x part
  , grouped_raw_symbol(
    L"\\[\\["
      L"(" TR_RE_TOKEN L")"
      L"(:#[0-9]+)?"
    L"\\]\\]"
  )

  // To test {{x<123>}}
  , encoded_symbol(
    L"{{"
      TR_RE_TOKEN
      L"<[-0-9<>]+>"
    L"}}"
  )

  // To capture {{x<123>}}'s <123> parts
  , grouped_encoded_symbol(
    L"{{"
      TR_RE_TOKEN
      L"(" L"<[-0-9<>]+>" L")"
    L"}}"
  )
;

}} // unnamed namespace rx

bool trsymbol::contains_raw_symbol(const std::wstring &s)
{
  return boost::algorithm::contains(s, L"[[")
      && cpp_regex_contains(s, rx::raw_symbol);
}

bool trsymbol::contains_encoded_symbol(const std::wstring &s)
{
  return boost::algorithm::contains(s, L"{{")
      && cpp_regex_contains(s, rx::encoded_symbol);
}

std::wstring trsymbol::create_symbol_target(const std::wstring &token, int id, int argc)
{
  std::wstring ret = L"{{";
  ret += token;
  ret += L'<';
  if (argc) {
    for (int i = 1; i <= argc; i++) {
      ret += L'\\';
      if (i < 10){
        ret += (L'0' + i);
      }else {
        ret += L'{';
        ret += std::to_wstring(i);
        ret += L'}';
      }
    }
  }
  ret += std::to_wstring(id);
  ret += L">}}";
  return ret;
}

int trsymbol::count_raw_symbols(const std::wstring &s)
{
  return !boost::algorithm::contains(s, L"[[") ? 0
       : ::cpp_regex_count(s, rx::raw_symbol);
}

std::wstring trsymbol::encode_symbol(const std::wstring &s)
{ return boost::regex_replace(s, rx::grouped_raw_symbol, L"{{$1\\([-0-9<>]+\\)}}"); }

// Example text to decode: 4<3<1>>,<2>
static std::wstring decode_symbol_stack(const wchar_t *str, const trsymbol::decode_fun_t &fun)
{
  std::stack<std::wstring> tokens;
  std::vector<std::wstring> args;
  while (wchar_t ch = *str++)
    if (ch == '<')
       tokens.push(std::wstring());
    else {
      if (::isdigit(ch) || ch == '-')
        if (int id = ::wcstol(str - 1, const_cast<wchar_t **>(&str), 10))
          if (*str++ == '>') { // start reduce
            while (!tokens.empty() && !tokens.top().empty()) {
              args.push_back(tokens.top());
              tokens.pop();
            }
            if (!tokens.empty()) {
              tokens.pop();
              std::wstring tok = fun(id, args);
              if (!tok.empty()) {
                tokens.push(tok);
                if (!args.empty())
                  args.clear();
                continue;
              }
            }
          }
      DERR("failed to decode symbol id");
      return std::wstring();
    }


  if (tokens.size() == 1)
    return tokens.top();

  DERR("failed to decode symbol stack");
  return std::wstring();
}

std::wstring trsymbol::decode_symbol(const std::wstring &text, const decode_fun_t &fun)
{
  return boost::regex_replace(text, rx::grouped_encoded_symbol, [&fun](const boost::wsmatch &m) {
    const std::wstring &matched_text = m[1];
    return decode_symbol_stack(matched_text.c_str(), fun);
  });
}

// EOF
