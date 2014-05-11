// engineloader.cc
// 4/26/2014 jichi
// List of all engines

#include "engine/engineloader.h"
#include "engine/model/majiro.h"
//#include "engine/model/kirikiri.h"

AbstractEngine *Engine::getEngine()
{
  if (MajiroEngine::match())
    return new MajiroEngine;
  //if (KiriKiriEngine::match())
  //  return new KiriKiriEngine;
  return nullptr;
}

// EOF
