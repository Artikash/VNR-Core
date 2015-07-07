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
    enableDynamicFont = true; // CreateFontIndirect only invoked once
    matchFiles << "BGI.*|BHVC.exe";
    //newLineString = "\n";
    attachFunction = &Self::attach;
  }
};

// EOF
