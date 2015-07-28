#pragma once

// mono.h
// 7/27/2015 jichi

#include "engine/enginemodel.h"

class MonoEngine : public EngineModel
{
  SK_EXTEND_CLASS(MonoEngine, EngineModel)
  static bool match();
  static bool attach();
public:
  MonoEngine()
  {
    name = "EmbedMono";
    encoding = Utf16Encoding;
    //newLineString = "\n";
    matchFunction = &Self::match;
    attachFunction = &Self::attach;
  }
};

// EOF
