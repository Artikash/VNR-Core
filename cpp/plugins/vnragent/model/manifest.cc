// manifest.cc
// 4/26/2014 jichi
// List of all engines

#include "model/manifest.h"
#include "model/engine/majiro.h"

AbstractEngine *Engine::getEngine()
{
  if (MajiroEngine::match())
    return new MajiroEngine;
  return nullptr;
}

// EOF
