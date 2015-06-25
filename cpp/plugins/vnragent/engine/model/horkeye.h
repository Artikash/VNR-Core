#pragma once

// horkeye.h
// 6/24/2015 jichi

#include "engine/enginemodel.h"

class HorkEyeEngine : public EngineModel
{
  SK_EXTEND_CLASS(HorkEyeEngine, EngineModel)
  static bool attach();
  static QString textFilter(const QString &text, int role);
  static QString translationFilter(const QString &text, int role);
public:
  HorkEyeEngine()
  {
    name = "EmbedHorkEye";
    enableDynamicEncoding = true;
    scenarioLineCapacity = 40;
    matchFiles << "arc0.dat" << "arc1.dat" << "arc2.dat" << "script.dat";
    newLineString = "[n]";
    attachFunction = &Self::attach;
    textFilterFunction = &Self::textFilter;
    translationFilterFunction = &Self::translationFilter;
  }
};

// EOF
