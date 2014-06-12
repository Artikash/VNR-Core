#pragma once

// bgi.h
// 5/11/2014 jichi

#include "engine/enginemodel.h"

class BGIEngine : public EngineModel
{
  SK_EXTEND_CLASS(BGIEngine, EngineModel)
  static ulong search(ulong startAddress, ulong stopAddress);
  static void hook(HookStack *stack);
public:
  BGIEngine()
  {
    name = "BGI";
    matchFiles << "BGI.*";
    searchFunction = &Self::search;
    hookFunction = &Self::hook;
  }
};

// EOF
