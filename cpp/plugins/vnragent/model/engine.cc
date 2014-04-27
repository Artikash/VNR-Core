// majiro.cc
// 4/20/2014 jichi

#include "model/engine.h"
#include "model/manifest.h"

/** Model */

AbstractEngine *AbstractEngine::getEngine()
{
  if (MajiroEngine::match())
    return new MajiroEngine;
  return nullptr;
}

// EOF
