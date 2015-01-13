// trrule.cc
// 9/20/2014 jichi

#include "trscript/trrule.h"
#include "cppjson/jsonescape.h"

#define SK_NO_QT
#define DEBUG "trrule.cc"
#include "sakurakit/skdebug.h"

// Helpers
namespace { // unnamed

inline std::string escape(const std::wstring &t)
{
  std::string r = cpp_json::escape_basic_string(t, true); // true = escape all chars
  if (r.find('\'') != std::string::npos)
    boost::replace_all(r, "'", "\\'");
  return r;
}

} // unnamed namespace

// Construction

void TranslationScriptRule::init(const TranslationScriptParam &param)
{
  id = param.id;
  source = param.source;
  target = param.target;

  if (param.f_regex) {
    flags |= RegexFlag;
    try {
      source_re = new boost::wregex(param.source);
    } catch (...) { // boost::bad_pattern
      DWOUT("invalid term: " << param.id << ", regex pattern: " << param.source);
      valid = false;
      return;
    }
  }
  valid = true; // must do this at the end
}

// Render

// A sample expected output without escape:
// <a href='json://{"type":"term","id":12345,"source":"pattern","target":"text"}'>pattern</a>
std::wstring TranslationScriptRule::render_target() const
{
  std::wstring ret = L"{\"type\":\"term\"";
  ret.append(L",\"id\":")
     .append(id);
  if (!is_regex()) { // do not save regex pattern to save memory
    if (!source.empty() && !::isdigit(source[0])) { // do not save escaped floating number
      std::string s = escape(source);
      ret.append(L",\"source\":\"")
         .append(s.cbegin(), s.cend())
         .push_back('"');
    }
    if (!target.empty()) {
      std::string s = escape(target);
      ret.append(L",\"target\":\"")
         .append(s.cbegin(), s.cend())
         .push_back('"');
    }
  }
  ret.push_back('}');

  ret.insert(0, L"<a href='json://");
  ret.push_back('\'');

  //if (!linkStyle.empty())
  //  ret.append(" style=\"")
  //     .append(linkStyle)
  //     .append("\"");

  ret.push_back('>');
  ret.append(target)
     .append(L"</a>");
  return ret;
}

// Replacement

void TranslationScriptRule::string_replace(std::wstring &ret, bool link) const
{
  if (boost::algorithm::contains(ret, source)) { // do not render_target if no match
    if (target.empty())
      boost::erase_all(ret, source);
    else
      boost::replace_all(ret, source,
          target.empty() ? std::wstring() : !link ? target : render_target());
  }
}

void TranslationScriptRule::regex_replace(std::wstring &ret, bool link) const
{
  //Q_ASSERT(source_re);
  try  {
    boost::wsmatch m; // search first, which has less opportunity to happen
    // match_default is the default value
    // format_all is needed to enable all features, but it is sligntly slower
    const auto &re = *source_re;
    if (boost::regex_search(ret, m, re))
      ret = boost::regex_replace(ret, re,
          target.empty() ? std::wstring() : !link ? target : render_target(),
          boost::match_default|boost::format_all);
  } catch (...) {
    DWOUT("invalid regex expression:" << target);
    valid = false;
  }
}

// EOF
