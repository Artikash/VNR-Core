// engineloader.cc
// 4/26/2014 jichi
// List of all engines

#include "engine/engineloader.h"
//#include "engine/model/cmvs.h"
#include "engine/model/majiro.h"

AbstractEngine *Engine::getEngine()
{
  //if (CMVSEngine::match())
  //  return new CMVSEngine;
  if (MajiroEngine::match())
    return new MajiroEngine;
  return nullptr;
}

// EOF
