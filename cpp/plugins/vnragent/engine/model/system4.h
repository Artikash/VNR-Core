#pragma once

// system4.h
// 5/25/2015 jichi

#include "engine/enginemodel.h"

// Single thread
class System4Engine : public EngineModel
{
  SK_EXTEND_CLASS(System4Engine, EngineModel)

  static bool attach();
  static ulong search(ulong startAddress, ulong stopAddress, int *type);
public:

  System4Engine()
  {
    name = "System4";
    wideChar = true; // UTF-16
    matchFiles << "AliceStart.ini";
    attachFunction = &Self::attach;
    //searchFunction = &Self::search; // not used
  }
};

// EOF
