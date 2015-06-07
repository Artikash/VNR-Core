#pragma once

// rgss.h
// 6/7/2015 jichi

#include "engine/enginemodel.h"

class RGSSEngine : public EngineModel
{
  SK_EXTEND_CLASS(RGSSEngine, EngineModel)
  static bool attach();
public:
  RGSSEngine()
  {
    name = "EmbedRGSS";
    encoding = Utf8Encoding;
    //matchFiles << "System/RGSS3*.dll";
    matchFiles << "Game.rgss3a";
    attachFunction = &Self::attach;
  }
};

// EOF
