// trdecoderule.cc
// 9/20/2014 jichi

#include "trcodec/trdecoderule.h"
#include "trcodec/trsymbol.h"
#include <boost/algorithm/string.hpp>
#include <boost/lambda/lambda.hpp>
//#include <QDebug>

//#define SK_NO_QT
//#define DEBUG "trdecoderule.cc"
//#include "sakurakit/skdebug.h"

// Helper functions

#ifdef WITH_LIB_TRRENDER
# include "trrender/trrender.h"
#endif // WITH_LIB_TRRENDER

static inline std::wstring _render(const std::wstring &target, int id)
{
#ifdef WITH_LIB_TRRENDER
  return ::tr_render_rule(target, id);
#else
  return L"<u>" + target + L"</u>";
#endif // WITH_LIB_TRRENDER
}

// Construction

void TranslationDecodeRule::init(const TranslationRule &param)
{
  id = param.id;
  flags = param.flags;
  category = param.category;
  target = param.target;

  if (trsymbol::contains_raw_symbol(target)) {
    source_symbol_count = trsymbol::count_raw_symbols(param.source);
    if (source_symbol_count)
      trsymbol::iter_raw_symbols(param.source, [this](const std::string &symbol) {
        if (!source_symbols.empty())
          source_symbols.push_back(source_symbol_sep);
        source_symbols += symbol;
      });
  }

  //  source = param.source;
  valid = true;
}

// Render

std::wstring TranslationDecodeRule::render_target(const std::vector<std::wstring> &args, bool mark) const
{
  std::wstring ret = target;
  if (!args.empty() && args.size() <= source_symbol_count) { // handle symbol here
    if (source_symbol_count == 1) // optimize if there is only one symbol
      boost::replace_all(ret, source_symbols, args.front());
    else {
      const auto symbol_splitter = boost::lambda::_1 == ',';
      std::vector<std::string> symbols;
      symbols.reserve(source_symbol_count);
      boost::split(symbols, source_symbols, symbol_splitter);
      //assert(symbols.size() == source_symbol_count);
      for (size_t i = 0; i < args.size(); i++)
        boost::replace_all(ret, symbols[i], args[i]);
    }
  }
  if (mark)
    ret = _render(ret, id);
  return ret;
}

// EOF
