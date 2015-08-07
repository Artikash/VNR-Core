#pragma once

// lcse.h
// 8/6/2015 jichi

#include "engine/enginemodel.h"

class LCScriptEngine : public EngineModel
{
  SK_EXTEND_CLASS(LCScriptEngine, EngineModel)
  static bool attach();
  //static QString rubyCreate(const QString &rb, const QString &rt);
  //static QString rubyRemove(const QString &text);
public:
  LCScriptEngine()
  {
    name = "EmbedLCScriptEngine";
    enableDynamicEncoding = true;
    //enableDynamicFont = true;
    newLineString = " "; // new line does not work
    matchFiles << "lcsebody*";
    attachFunction = &Self::attach;
    //textSeperators << "\x01" << "\x02\x03";
    //rubyCreateFunction = &Self::rubyCreate;
    //rubyRemoveFunction = &Self::rubyRemove;
  }
};

// EOF
