#pragma once

// luna.h
// 8/2/2015 jichi

#include "engine/enginemodel.h"

class LunaSoftEngine : public EngineModel
{
  SK_EXTEND_CLASS(LunaSoftEngine, EngineModel)
  static bool attach();
  //static QString rubyCreate(const QString &rb, const QString &rt);
  //static QString rubyRemove(const QString &text);
public:
  LunaSoftEngine()
  {
    name = "EmbedLunaSoft";
    enableDynamicEncoding = true;
    enableDynamicFont = true;
    matchFiles << "Pac/*.pac";
    //newLineString = "\n";
    scenarioLineCapacity = 40; // around 50 in 悪堕ラビリンス
    attachFunction = &Self::attach;
    //rubyCreateFunction = &Self::rubyCreate;
    //rubyRemoveFunction = &Self::rubyRemove;
  }
};

// EOF
