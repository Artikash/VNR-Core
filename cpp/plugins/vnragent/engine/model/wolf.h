#pragma once

// wolf.h
// 6/9/2015 jichi

#include "engine/enginemodel.h"

class WolfRPGEngine : public EngineModel
{
  SK_EXTEND_CLASS(WolfRPGEngine, EngineModel)
  static bool attach();
  //static QString textFilter(const QString &text, int role);
public:
  WolfRPGEngine()
  {
    name = "EmbedWolfRPG";
    dynamicEncoding = true;
    matchFiles << "data.wolf|data/*.wolf";
    attachFunction = &Self::attach;
    //textFilterFunction = &Self::textFilter;
  }
};

// EOF
