#pragma once

// debonosu.h
// 6/18/2015 jichi

#include "engine/enginemodel.h"

class DebonosuEngine : public EngineModel
{
  SK_EXTEND_CLASS(DebonosuEngine, EngineModel)
  static bool attach();
  static QString textFilter(const QString &text, int role);
public:
  DebonosuEngine()
  {
    name = "EmbedDebonosu";
    enableDynamicEncoding = true;
    enableLocaleEmulation = true; // fix thread codepage in MultiByteToWideChar
    matchFiles << "bmp.pak" << "dsetup.dll";
    attachFunction = &Self::attach;
    textFilterFunction = &Self::textFilter;
  }
};

// EOF
