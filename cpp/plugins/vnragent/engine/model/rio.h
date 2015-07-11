#pragma once

// rio.h
// 7/10/2015 jichi

#include "engine/enginemodel.h"

class ShinaRioEngine : public EngineModel
{
  SK_EXTEND_CLASS(ShinaRioEngine, EngineModel)
  static bool attach();
  static QString textFilter(const QString &text, int role);

public:
  ShinaRioEngine()
  {
    name = "EmbedRio";
    enableDynamicEncoding = true;
    newLineString = "_r";
    matchFiles << "rio.ini";
    scenarioLineCapacity =
    otherLineCapacity = 40; // 60 in 3rd games (30 wide characters)
    attachFunction = &Self::attach;
    textFilterFunction = &Self::textFilter;
  }
};

// EOF
