#pragma once

// enginesettings.h
// 4/20/2014 jichi

#include "engine/enginedef.h"

class EngineSettings
{
public:
  bool enabled
     , detectsControl
     , nameTextVisible
     , textVisible[Engine::RoleCount]
     , transcodingEnabled[Engine::RoleCount]
     , translationEnabled[Engine::RoleCount]
     , extractionEnabled[Engine::RoleCount]
     ;

  long scenarioSignature,
       nameSignature;

  // Set all properties to false
  EngineSettings()
    : enabled(false)
    , detectsControl(false)
    , nameTextVisible(false)
    , scenarioSignature(0)
    , nameSignature(0)
  {
    for (int role = 0; role < Engine::RoleCount; role++)
      textVisible[role]
      = transcodingEnabled[role]
      = translationEnabled[role]
      = extractionEnabled[role]
      = false;
  }

  Engine::TextRole textRoleOf(long signature) const
  {
    return !signature ? Engine::OtherRole :
        signature == scenarioSignature ? Engine::ScenarioRole :
        signature == nameSignature ? Engine::NameRole :
        Engine::OtherRole;
  }
};

// EOF
