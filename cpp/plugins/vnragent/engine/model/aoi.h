#pragma once

// aoi.h
// 6/6/2015 jichi

#include "engine/enginemodel.h"

class SystemAoiEngine : public EngineModel
{
  SK_EXTEND_CLASS(SystemAoiEngine, EngineModel)
  bool attach();
public:
  SystemAoiEngine()
  {
    name = "EmbedSystemAoi";
    matchFiles << "Ags*.dll" << "Aoi*.dll";
    textSeperators << "[u]";
    //newLineString = "\n";
    attachFunction = std::bind(&Self::attach, this);
  }
};

// EOF
