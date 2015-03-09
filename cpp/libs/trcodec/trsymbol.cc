// trsymbol.cc
// 3/5/2015 jichi
#include "trcodec/trsymbol.h"
#include "trcodec/trdefine.h"
#include "cpputil/cppregex.h"
#include "cpputil/cppstring.h"
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
  , raw_symbol_with_token_group(
    L"\\[\\["
      L"(" TR_RE_TOKEN L")"
      L"(:#[0-9]+)?"
    L"\\]\\]"
  )

  // To capture [[x#123]]'s x#123 part
  , raw_symbol_with_symbol_group(
    L"\\[\\["
      L"("
        TR_RE_TOKEN
        L"(:#[0-9]+)?"
      L")"
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
  , encoded_symbol_with_param_group(
    L"{{"
      TR_RE_TOKEN
      L"(" L"<[-0-9<>]+>" L")"
    L"}}"
  )
;

}} // unnamed namespace rx

bool trsymbol::contains_raw_symbol(const std::wstring &s)
{
  return boost::algorithm::contains(s, "[[")
      && cpp_regex_contains(s, rx::raw_symbol);
}

bool trsymbol::contains_encoded_symbol(const std::wstring &s)
{
  return boost::algorithm::contains(s, "{{")
      && cpp_regex_contains(s, rx::encoded_symbol);
}

std::string trsymbol::create_symbol_target(const std::string &token, int id, int argc)
{
  std::string ret = "{{";
  ret += token;
  ret += '<';
  if (argc) {
    for (int i = 1; i <= argc; i++) {
      ret.push_back('\\'); // must be '\\' instead of '$' to avoid using {}
      if (i < 10)
        ret.push_back('0' + i);
      else {
        ret += '{';
        ret += std::to_string(i);
        ret += '}';
      }
    }
  }
  ret += std::to_string(id);
  ret += ">}}";
  return ret;
}

size_t trsymbol::count_raw_symbols(const std::wstring &s)
{
  return !boost::algorithm::contains(s, "[[") ? 0
       : ::cpp_regex_count(s, rx::raw_symbol);
}

std::wstring trsymbol::encode_symbol(const std::wstring &s)
{ return boost::regex_replace(s, rx::raw_symbol_with_token_group, "{{$1\\([-0-9<>]+\\)}}"); }

void trsymbol::iter_raw_symbols(const std::wstring &target, const collect_string_f &fun)
{
  std::for_each(
    boost::wsregex_iterator(target.cbegin(), target.cend(), rx::raw_symbol_with_symbol_group),
    boost::wsregex_iterator(),
    [&fun](const boost::wsmatch &m) {
      std::string matched_text = ::cpp_string_of(m.str());
      fun(matched_text);
    }
  );
}

// Example text to decode: 4<3<1>>,<2>
static std::wstring decode_symbol_stack(const char *str, const trsymbol::decode_f &fun)
{
  std::vector<std::wstring> args;

  {
    // Optimization for the first case <id>
    const char *pos;
    if (int id = ::strtol(str + 1, const_cast<char **>(&pos), 10))
      if (pos[0] == '>' && pos[1] == 0)
        return fun(id, args);
  }

  std::stack<std::wstring> tokens;
  while (char ch = *str++)
    if (ch == '<')
       tokens.push(std::wstring());
    else {
      if (::isdigit(ch) || ch == '-')
        if (int id = ::strtol(str - 1, const_cast<char **>(&str), 10))
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

std::wstring trsymbol::decode_symbol(const std::wstring &text, const decode_f &fun)
{
  return boost::regex_replace(text, rx::encoded_symbol_with_param_group, [&fun](const boost::wsmatch &m) {
    std::string matched_text = ::cpp_string_of(m.str(1));
    return decode_symbol_stack(matched_text.c_str(), fun);
  });
}

// EOF
