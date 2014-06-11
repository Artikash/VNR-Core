#pragma once

// bgi.h
// 5/11/2014 jichi

#include "engine/enginemodel.h"

struct BGIEngine : EngineModel
{
public:
  BGIEngine()
    : name("BGI")
    , matchFiles("BGI.*")
    , searchFunction(search)
    , hookFunction(hook)
  {}

private:
  ulong search(ulong startAddress, stopAddress);
  void hook(HookStack *stack);
};

// EOF
