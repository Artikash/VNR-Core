// trencoderule.cc
// 9/20/2014 jichi

#include "trcodec/trencoderule.h"
#include "trcodec/trsymbol.h"
//#include <QDebug>

#define SK_NO_QT
#define DEBUG "trencoderule.cc"
#include "sakurakit/skdebug.h"

// Construction

void TranslationEncodeRule::init(const TranslationRule &param)
{
  id = param.id;
  flags = param.flags;
  category = param.category;
  source = param.source;
  if (!param.target.empty())
    token = param.token;

  try {
    init_source();
  } catch (...) { // boost::bad_pattern
    DWOUT("invalid term: " << param.id << ", regex pattern: " << param.source);
    return;
  }

  valid = true; // do this at last to prevent crash
}

// Initialization

void TranslationEncodeRule::cache_target() const
{
  if (target.empty() && !token.empty())
    target = trsymbol::render_symbol(token, id);
}

void TranslationEncodeRule::init_source()
{
  if (is_symbolic()) {
    source;
    set_regex(true);
  }
  if (is_regex()) {
    if (is_icase())
      source_re = new boost::wregex(source, boost::wregex::icase);
    else
      source_re = new boost::wregex(source);
    source.clear(); // no longer needed any more
  }
}

// Replacement

void TranslationEncodeRule::string_replace(std::wstring &ret) const
{
  if (token.empty()) {
    if (is_icase())
      boost::ierase_all(ret, source);
    else
      boost::erase_all(ret, source);
  } else {
    cache_target();
    if (is_icase())
      boost::ireplace_all(ret, source, target);
    else
      boost::replace_all(ret, source, target);
  }
}

void TranslationEncodeRule::regex_replace(std::wstring &ret) const
{
  try  {
    // match_default is the default value
    // format_all is needed to enable all features, but it is sligntly slower
    cache_target();
    ret = boost::regex_replace(ret, *source_re, target,
        boost::match_default|boost::format_all);
  } catch (boost::regex_error &e) {
    DWOUT("invalid term: " << id << ", what: " << e.what() << ", regex pattern: " << source);
    valid = false;
  }
}

bool TranslationEncodeRule::regex_exists(const std::wstring &t) const
{
  try  {
    boost::wsmatch m; // search first, which has less opportunity to happen
    return boost::regex_search(t, m, *source_re);
  } catch (boost::regex_error &e) {
    DWOUT("invalid term: " << id << ", what: " << e.what() << ", regex pattern: " << source);
    valid = false;
    return false;
  }
}

// EOF
