#pragma once

// gxp.h
// 8/15/2015 jichi

#include "engine/enginemodel.h"

class GXPEngine : public EngineModel
{
  SK_EXTEND_CLASS(GXPEngine, EngineModel)
  static bool attach();

public:
  GXPEngine()
  {
    name = "EmbedGXP";
    matchFiles << "*.gxp";
    encoding = Utf16Encoding;
    //enableDynamicFont = true;
    newLineString = "%r";
    attachFunction = &Self::attach;
  }
};

// EOF
