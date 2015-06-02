#pragma once

// age.h
// 6/1/2014 jichi

#include "engine/enginemodel.h"

class ARCGameEngine : public EngineModel
{
  SK_EXTEND_CLASS(ARCGameEngine, EngineModel)
  static bool attach();
public:
  ARCGameEngine() //BlockingAttribute|SingleThreadAttribute)
  {
    name = "EmbedARCGameEngine";
    matchFiles << "AGERC.DLL"; // the process name is AGE.EXE.
    attachFunction = &Self::attach;
  }
};

// EOF
