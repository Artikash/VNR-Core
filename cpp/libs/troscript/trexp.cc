// trexp.cc
// 5/17/2015 jichi
#include "troscript/trexp.h"
#include "trsym/trsymdef.h"
#include "trsym/trsym.h"
#include "cpputil/cppregex.h"
#include "cpputil/cppstring.h"
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
//#include <QDebug>

#define SK_NO_QT
#define DEBUG "trencode.cc"
#include "sakurakit/skdebug.h"

#define SYMBOL_ESCAPE_RE L"\\|?!.+*^<>(){}" // characters needed to be escaped for a regex, except [] and $

// Precompiled regex
namespace { namespace rx {

const boost::wregex
  // To test [[x#123]]
  raw_symbol(
    L"\\[\\["
      TRCODEC_RE_TOKEN
      L"(?:#[0-9]+)?"
    L"\\]\\]"
  )

  // To capture [[x#123]]'s x part
  , raw_symbol_with_token_group(
    L"\\[\\["
      L"(" TRCODEC_RE_TOKEN L")"
      L"(?:#[0-9]+)?"
    L"\\]\\]"
  )

  // To capture [[x#123]]'s both x and #123 part
  , raw_symbol_with_symbol_group(
    L"\\[\\["
      L"("
        TRCODEC_RE_TOKEN
        L"(?:#[0-9]+)?"
      L")"
    L"\\]\\]"
  )

  // To test {{x<123>}}
  , encoded_symbol(
    L"{{"
      TRCODEC_RE_TOKEN
      L"<[-0-9<>]+>"
    L"}}"
  )
;

}} // unnamed namespace rx

// Compile pattern

/// Return whether a text contains regex special chars except '[]'
static inline bool _symbol_needs_escape_re(const std::wstring &s)
{ return std::wstring::npos != s.find_first_of(SYMBOL_ESCAPE_RE); }

/// Escape regex special chars. FIXME: $ is not escaped
static inline std::wstring _symbol_escape_re(const wchar_t *s)
{
  std::wstring ret;
  while (wchar_t c = *s++) {
    if (::wcschr(SYMBOL_ESCAPE_RE, c))
      ret.push_back(L'\\');
    ret.push_back(c);
  }
  return ret;
}

static inline std::wstring _symbol_escape_re(const std::wstring &s)
{ return _symbol_escape_re(s.c_str()); }

#define _ENCODE_SYMBOL_MATCH \
"\\(" \
  "{{" \
    "$1" \
    "(?:" "[_$]" TRCODEC_RE_TOKEN_A ")?" \
    "<[-0-9<>]+>" \
  "}}" \
"\\)"
static std::wstring _encode_symbol_match(const boost::wsmatch &m)
{
  std::wstring ret = L"(" L"{{";
  std::wstring tokens = m[1];
  if (tokens.find(',') == std::wstring::npos)
    ret += tokens;
  else {
    std::replace(tokens.begin(), tokens.end(), L',', L'|');
    ret += L"(?:";
    ret += tokens;
    ret += L')';
  }
  ret +=
    L"(?:" L"[_$]" TRCODEC_RE_TOKEN L")?"
    L"[-0-9<>]+"
  L"}}" L")";
  return ret;
}

std::wstring trexp::compile_pattern(const std::wstring &s, bool escape)
{
  if (s.find(',') == std::wstring::npos) {
    if (escape && _symbol_needs_escape_re(s))
      return boost::regex_replace(_symbol_escape_re(s), rx::raw_symbol_with_token_group, _ENCODE_SYMBOL_MATCH);
    else
      return boost::regex_replace(s, rx::raw_symbol_with_token_group, _ENCODE_SYMBOL_MATCH);
  } else {
    if (escape && _symbol_needs_escape_re(s))
      return boost::regex_replace(_symbol_escape_re(s), rx::raw_symbol_with_token_group, _encode_symbol_match);
    else
      return boost::regex_replace(s, rx::raw_symbol_with_token_group, _encode_symbol_match);
  }
}

// Compile format

///////////////////////////////

std::wstring trexp::compile_format(const std::wstring &target, const std::wstring &source)
{
  size_t source_symbol_count = trsym::count_raw_symbols(source);
  if (!source_symbol_count)
    return target;
  std::wstring ret = target;
  if (source_symbol_count == 1) { // optimize if there is only one symbol, replace between "[[", "]]" by "$1"
    size_t start = ret.find(L"[[");
    if (start != std::wstring::npos) {
      size_t stop = ret.find(L"]]", start);
      if (stop != std::wstring::npos)
        ret.replace(start, stop - start + 2, L"$1"); // 2 = len("]]")
    }
  } else {
    std::vector<std::string> symbols;
    symbols.reserve(source_symbol_count);
    trsym::iter_raw_symbols(source, [&symbols](const std::string &it) {
      symbols.push_back(it);
    });
    for (size_t i = 0; i < symbols.size(); i++) {
      std::string repl = "$" + boost::lexical_cast<std::string>(i+1);
      boost::replace_all(ret, symbols[i], repl);
    }

    if (source.find('#') != std::wstring::npos && ret.find('#') != std::wstring::npos) // there might be unhandled '#'
      for (size_t i = 0; i < symbols.size(); i++) {
        size_t pos = symbols[i].find('#');
        if (pos != std::wstring::npos) {
          std::string pat = "[[" + symbols[i].substr(pos),
                      repl = "$" + boost::lexical_cast<std::string>(i+1);
          boost::replace_all(ret, pat, repl);
        }
      }
  }
  return ret;
}

// EOF
