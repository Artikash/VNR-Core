#pragma once

// eushully.h
// 6/1/2014 jichi

#include "engine/enginemodel.h"

struct EushullyEngine : EngineModel
{
  EushullyEngine() //BlockingAttribute|SingleThreadAttribute)
    : name("Eushully")
    , matchFiles("AGERC.DLL") // the process name is AGE.EXE.
    , searchFunction(search)
    , hookFunction(hook)
  {}

private:
  static ulong search(ulong startAddress, ulong stopAddress);
  static void hook(HookStack *stack);
};

// EOF
