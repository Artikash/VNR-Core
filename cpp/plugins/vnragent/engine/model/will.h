#pragma once

// will.h
// 7/3/2015 jichi

#include "engine/enginemodel.h"

class WillPlusEngine : public EngineModel
{
  SK_EXTEND_CLASS(WillPlusEngine, EngineModel)
  static bool attach();

public:
  WillPlusEngine()
  {
    name = "EmbedWillPlus";
    encoding = Utf16Encoding;
    //enableDynamicEncoding = true;
    newLineString = "\\n";
    matchFiles << "Rio.arc" << "Chip*.arc";
    attachFunction = &Self::attach;
  }
};

// EOF
