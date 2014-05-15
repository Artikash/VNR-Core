#pragma once

// enginesettings.h
// 4/20/2014 jichi

#include "engine/enginedef.h"

class EngineSettings
{
public:
  bool enabled          // if vnragent is enabled
     , detectsControl   // be aware if CTRL is pressed
     , extractsAllTexts // send all texts unless not enabled
     , nameTextVisible  // display both translation and the original text for names
     , textVisible[Engine::RoleCount]           // if display/hide all texts
     , transcodingEnabled[Engine::RoleCount]    // if fix the encoding
     , translationEnabled[Engine::RoleCount]    // if display translation
     , extractionEnabled[Engine::RoleCount]     // if send text
     ;

  long scenarioSignature,
       nameSignature;

  // Set all properties to false
  EngineSettings()
    : enabled(false)
    , detectsControl(false)
    , extractsAllTexts(false)
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
