// trdecoderule.cc
// 9/20/2014 jichi

#include "trcodec/trdecoderule.h"
#include "trcodec/trsymbol.h"
//#include <QDebug>

#define SK_NO_QT
#define DEBUG "trdecoderule.cc"
#include "sakurakit/skdebug.h"


// Construction

void TranslationDecodeRule::init(const TranslationRule &param)
{
  id = param.id;
  flags = param.flags;
  category = param.category;
  token = param.token;
  target = param.target;
  valid = true;
}

// Initialization

std::wstring TranslationDecodeRule::render(const std::vector<std::wstring> &args, bool mark) const
{
  return target;
}

// EOF
