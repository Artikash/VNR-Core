#pragma once

// bgi.h
// 5/11/2014 jichi

#include "engine/enginemodel.h"

class BGIEngine : public EngineModel
{
  SK_EXTEND_CLASS(BGIEngine, EngineModel)
  static bool attach();
public:
  BGIEngine()
  {
    name = "EmbedBGI";
    enableDynamicEncoding = true;
    matchFiles << "BGI.*";
    //newLineString = nullptr; // TODO: not verified
    attachFunction = &Self::attach;
  }
};

// EOF
