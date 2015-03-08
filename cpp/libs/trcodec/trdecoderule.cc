// trdecoderule.cc
// 9/20/2014 jichi

#include "trcodec/trdecoderule.h"
#include "trcodec/trsymbol.h"
#include <QDebug>

//#define SK_NO_QT
//#define DEBUG "trdecoderule.cc"
//#include "sakurakit/skdebug.h"

// Construction

void TranslationDecodeRule::init(const TranslationRule &param)
{
  id = param.id;
  flags = param.flags;
  category = param.category;
  target = param.target;
  if (trsymbol::contains_raw_symbol(target)) // only need do this when containing symbol
    source = param.source;
  valid = true;
}

// Initialization

std::wstring TranslationDecodeRule::render(const std::vector<std::wstring> &args, bool mark) const
{
  std::wstring ret = target;
  if (!source.empty()) {
    // handle symbol here
  }
  if (mark) {
    // render target with underlined here
  }
  return ret;
}

// EOF
