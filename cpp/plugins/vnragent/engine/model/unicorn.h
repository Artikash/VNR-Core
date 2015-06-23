#pragma once

// unicorn.h
// 6/22/2015 jichi

#include "engine/enginemodel.h"

class UnicornEngine : public EngineModel
{
  SK_EXTEND_CLASS(UnicornEngine, EngineModel)
  static bool attach();
public:
  UnicornEngine()
  {
    name = "EmbedUnicorn";
    enableDynamicEncoding = true;
    matchFiles << "*.szs|data/*.szs";
    attachFunction = &Self::attach;
  }
};

// EOF
