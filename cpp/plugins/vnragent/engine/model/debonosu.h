#pragma once

// debonosu.h
// 6/18/2015 jichi

#include "engine/enginemodel.h"

class DebonosuEngine : public EngineModel
{
  SK_EXTEND_CLASS(DebonosuEngine, EngineModel)
  static bool attach();
public:
  DebonosuEngine()
  {
    name = "EmbedDebonosu";
    dynamicEncoding = true;
    matchFiles << "bmp.pak" << "dsetup.dll";
    attachFunction = &Self::attach;
  }
};

// EOF
