#pragma once

// bgi2.h
// 8/5/2014 jichi

#include "engine/enginemodel.h"

class BGI2Engine : public EngineModel
{
  SK_EXTEND_CLASS(BGI2Engine, EngineModel)
  static ulong search(ulong startAddress, ulong stopAddress);
  static void hook(HookStack *stack);
public:
  BGI2Engine()
  {
    name = "BGI2";
    matchFiles << "BGI.*";
    searchFunction = &Self::search;
    hookFunction = &Self::hook;
  }
};

// EOF
