#pragma once

// lova.h
// 7/19/2015 jichi

#include "engine/enginemodel.h"

class LovaEngine : public EngineModel
{
  SK_EXTEND_CLASS(LovaEngine, EngineModel)
  static bool attach();
public:
  LovaEngine()
  {
    name = "EmbedLova";
    encoding = Utf8Encoding;
    matchFiles << "awesomium_process.exe" << "UE3ShaderCompileWorker.exe";
    newLineString = nullptr; // <br/> does not work
    attachFunction = &Self::attach;
  }
};

// EOF
