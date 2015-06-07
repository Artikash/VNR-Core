#pragma once

// aoi.h
// 6/6/2015 jichi

#include "engine/enginemodel.h"

class SystemAoiWEngine : public EngineModel
{
  SK_EXTEND_CLASS(SystemAoiWEngine, EngineModel)
  static bool attach();
public:
  SystemAoiWEngine()
  {
    name = "EmbedSystemAoiW";
    encoding = Utf16Encoding;
    matchFiles << "Ags*.dll" << "Aoi*.dll";
    attachFunction = &Self::attach;
  }
};

// EOF
