#pragma once

// eushully.h
// 6/1/2014 jichi

#include "engine/enginemodel.h"

class EushullyEngine : public EngineModel
{
  SK_EXTEND_CLASS(EushullyEngine, EngineModel)
  static ulong search(ulong startAddress, ulong stopAddress);
  static void hook(HookStack *stack);
public:
  EushullyEngine() //BlockingAttribute|SingleThreadAttribute)
  {
    name = "Eushully";
    matchFiles << "AGERC.DLL"; // the process name is AGE.EXE.
    searchFunction = &Self::search;
    hookFunction = &Self::hook;
  }
};

// EOF
