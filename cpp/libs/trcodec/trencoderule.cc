// trencoderule.cc
// 9/20/2014 jichi

#include "trcodec/trencoderule.h"
#include "trcodec/trescape.h"
#include <QDebug>

#define SK_NO_QT
#define DEBUG "trencoderule.cc"
#include "sakurakit/skdebug.h"

// Construction

void TranslationEncodeRule::init(const param_type &param, bool precompile_regex)
{
  id = param.id;
  category = param.category;
  source = param.source;
  target = param.target;

  if (param.f_icase)
    flags |= IcaseFlag;

  if (param.f_regex) {
    flags |= RegexFlag;
    if (precompile_regex)
      try {
        cache_re();
      } catch (...) { // boost::bad_pattern
        DWOUT("invalid term: " << param.id << ", regex pattern: " << param.source);
        valid = false;
        return;
      }
  }

  valid = true; // must do this at the end
}

// Render

std::wstring TranslationEncodeRule::render_target() const
{
  return target;
}

// Replacement

void TranslationEncodeRule::string_replace(std::wstring &ret) const
{
  if (target.empty()) {
    if (is_icase())
      boost::ierase_all(ret, source);
    else
      boost::erase_all(ret, source);
  } else {
    if (is_icase())
      boost::ireplace_all(ret, source, render_target());
    else
      boost::replace_all(ret, source, render_target());
  }
}

void TranslationEncodeRule::regex_replace(std::wstring &ret) const
{
  try  {
    // match_default is the default value
    // format_all is needed to enable all features, but it is sligntly slower
    cache_re();
    ret = boost::regex_replace(ret, *source_re,
        target.empty() ? std::wstring() : render_target(),
        boost::match_default|boost::format_all);
  } catch (...) {
    DWOUT("invalid term: " << id << ", regex pattern: " << source);
    valid = false;
  }
}

bool TranslationEncodeRule::regex_exists(const std::wstring &t) const
{
  try  {
    cache_re();
    boost::wsmatch m; // search first, which has less opportunity to happen
    return boost::regex_search(t, m, *source_re);
  } catch (...) {
    DWOUT("invalid term: " << id << ", regex pattern: " << source);
    valid = false;
    return false;
  }
}

// EOF
