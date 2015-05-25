#pragma once

// system4.h
// 5/25/2015 jichi

#include "engine/enginemodel.h"

// Single thread
class System4Engine : public EngineModel
{
  SK_EXTEND_CLASS(System4Engine, EngineModel)
  static ulong search(ulong startAddress, ulong stopAddress);
  static void hook(HookStack *stack);
public:
  System4Engine()
  {
    name = "System4";
    matchFiles << "AliceStart.ini";
    searchFunction = &Self::search;
    hookFunction = &Self::hook;
  }
};

// EOF
