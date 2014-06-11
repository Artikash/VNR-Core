// engineloader.cc
// 4/26/2014 jichi
// List of all engines

#include "engine/engineloader.h"
#include "engine/model/bgi.h"
#include "engine/model/circus.h"
#include "engine/model/elf.h"
#include "engine/model/eushully.h"
#include "engine/model/majiro.h"
#include "engine/model/siglus.h"

AbstractEngine *Engine::getEngine()
{
  if (BGIEngine::match()) return new BGIEngine;
  if (CircusEngine::match()) return new CircusEngine;
  if (ElfEngine::match()) return new ElfEngine;
  if (EushullyEngine::match()) return new EushullyEngine;
  if (MajiroEngine::match()) return new MajiroEngine;
  if (SiglusEngine::match()) return new SiglusEngine;
  return nullptr;
}

// EOF
