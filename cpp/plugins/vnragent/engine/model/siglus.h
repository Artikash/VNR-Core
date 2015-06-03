#pragma once

// siglus.h
// 5/25/2014 jichi

#include "engine/enginemodel.h"

// Single thread
class SiglusEngine : public EngineModel
{
  SK_EXTEND_CLASS(SiglusEngine, EngineModel)
  static bool attach();
public:
  SiglusEngine()
  {
    name = "EmbedSiglusEngine";
    encoding = Utf16Encoding;
    matchFiles << "SiglusEngine.exe";
    attachFunction = &Self::attach;
  }
};

// EOF
