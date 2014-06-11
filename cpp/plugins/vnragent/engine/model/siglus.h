#pragma once

// siglus.h
// 5/25/2014 jichi

#include "engine/enginemodel.h"

// Single thread
struct SiglusEngine : EngineModel
{
  SiglusEngine()
    : name("SiglusEngine")
    , wideChar(true)
    , matchFiles("SiglusEngine.exe")
    , attachFunction(attach)
    //, searchFunction(search) // not used
  {}

private:
  static bool attach();
  static ulong search(ulong startAddress, ulong stopAddress);
};

// EOF
