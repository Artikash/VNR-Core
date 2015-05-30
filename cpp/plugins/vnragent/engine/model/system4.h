#pragma once

// system4.h
// 5/25/2015 jichi

#include "engine/enginemodel.h"

// Single thread
class System4Engine : public EngineModel
{
  SK_EXTEND_CLASS(System4Engine, EngineModel)
  static bool attach();
public:
  System4Engine()
  {
    name = "System4";
    wideChar = false;
    matchFiles << "AliceStart.ini";
    attachFunction = &Self::attach;
  }
};

// EOF
