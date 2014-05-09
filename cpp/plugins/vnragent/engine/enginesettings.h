#pragma once

// enginesettings.h
// 4/20/2014 jichi

#include "engine/enginedef.h"

class EngineSettings
{
public:
  bool enabled
     , nameTextVisible
     , textVisible[Engine::RoleCount]
     , transcodingEnabled[Engine::RoleCount]
     , translationEnabled[Engine::RoleCount]
     , extractionEnabled[Engine::RoleCount]
     ;

  // Set all properties to false
  EngineSettings()
    : enabled(false)
    , nameTextVisible(false)
  {
    for (int role = 0; role < Engine::RoleCount; role++)
      textVisible[role]
      = transcodingEnabled[role]
      = translationEnabled[role]
      = extractionEnabled[role]
      = false;
  }
};

// EOF
