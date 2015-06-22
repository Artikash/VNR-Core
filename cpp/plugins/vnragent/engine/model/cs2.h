#pragma once

// cs2.h
// 6/21/2015 jichi

#include "engine/enginemodel.h"

class CatSystem2Engine : public EngineModel
{
  SK_EXTEND_CLASS(CatSystem2Engine, EngineModel)
  static bool attach();
  static QString textFilter(const QString &text, int role);
public:
  CatSystem2Engine()
  {
    name = "EmbedCatSystem2";
    enableDynamicEncoding = true;
    enableLocaleEmulation = true; // fix thread codepage in MultiByteToWideChar
    matchFiles << "bmp.pak" << "dsetup.dll";
    attachFunction = &Self::attach;
    textFilterFunction = &Self::textFilter;
  }
};

// EOF
