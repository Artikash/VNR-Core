#pragma once

// siglus.h
// 5/25/2014 jichi

#include "engine/enginemodel.h"

// Single thread
class SiglusEngine : public EngineModel
{
  SK_EXTEND_CLASS(SiglusEngine, EngineModel)
  static bool attach();
  static ulong search(ulong startAddress, ulong stopAddress);
public:
  SiglusEngine()
  {
    name = "SiglusEngine";
    wideChar = true; // UTF-16
    matchFiles << "SiglusEngine.exe";
    attachFunction = &Self::attach;
    //searchFunction = &Self::search; // not used
  }
};

// EOF
