#pragma once

// cs2.h
// 6/21/2015 jichi

#include "engine/enginemodel.h"

class CatSystem2Engine : public EngineModel
{
  SK_EXTEND_CLASS(CatSystem2Engine, EngineModel)
  static bool attach();
public:
  CatSystem2Engine()
  {
    name = "EmbedCatSystem2";
    enableDynamicEncoding = true;
    matchFiles << "*.int";
    attachFunction = &Self::attach;
  }
};

// EOF
