// trencoderule.cc
// 9/20/2014 jichi

#include "trcodec/trencoderule.h"
#include "trcodec/trsymbol.h"
#include "cpputil/cppregex.h"
//#include <QDebug>

#define SK_NO_QT
#define DEBUG "trencoderule.cc"
#include "sakurakit/skdebug.h"

#define WITH(...) \
  try { \
    __VA_ARGS__ \
  } catch (boost::regex_error &e) { \
    DWERR("invalid term: " << id << ", what: " << e.what() << ", regex pattern: " << source); \
    valid = false; \
  } catch (...) { \
    DWERR("invalid term: " << id << ", regex pattern: " << source); \
    valid = false; \
  }

// Construction

void TranslationEncodeRule::init(const TranslationRule &param)
{
  id = param.id;
  flags = param.flags;
  category = param.category;
  source = param.source;
  if (!param.target.empty())
    target_token = param.token;

  source_symbol_count = trsymbol::count_raw_symbols(source);
  WITH (
    init_source();
    valid = true; // do this at last to prevent crash
  )
}

// Initialization

void TranslationEncodeRule::init_target()
{ target = trsymbol::create_symbol_target(target_token, id, source_symbol_count); }

void TranslationEncodeRule::init_source()
{
  if (is_symbolic()) {
    bool escape = !is_regex();
    source = trsymbol::encode_symbol(source, escape);
    set_regex(true);
  }
  if (is_regex()) {
    source_re = new boost::wregex(source,
      is_icase() ? boost::wregex::perl|boost::wregex::icase
                 : boost::wregex::perl
    );
    source.clear(); // no longer needed any more
  }
}

// Replacement

void TranslationEncodeRule::string_replace(std::wstring &ret) const
{
  if (target_token.empty()) {
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
  WITH (
    // match_default is the default value
    // format_all is needed to enable all features, but it is sligntly slower
    cache_target();
    ret = boost::regex_replace(ret, *source_re, target,
        boost::match_default|boost::format_all);
  )
}

bool TranslationEncodeRule::regex_exists(const std::wstring &t) const
{
  WITH (
    return ::cpp_regex_contains(t, *source_re);
  )
  return false;
}

// EOF