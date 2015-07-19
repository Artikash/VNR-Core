#pragma once

// pal.h
// 7/18/2015 jichi

#include "engine/enginemodel.h"

class PalEngine : public EngineModel
{
  SK_EXTEND_CLASS(PalEngine, EngineModel)
  static bool attach();
  static QString textFilter(const QString &text, int role);

public:
  PalEngine()
  {
    name = "EmbedPal";
    //enableDynamicEncoding = true;
    newLineString = nullptr;
    matchFiles << "dll/Pal.dll";
    attachFunction = &Self::attach;
    textFilterFunction = &Self::textFilter;
  }
};

// EOF
