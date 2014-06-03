// engineloader.cc
// 4/26/2014 jichi
// List of all engines

#include "engine/engineloader.h"
#include "engine/model/bgi.h"
#include "engine/model/eushully.h"
#include "engine/model/majiro.h"
#include "engine/model/siglus.h"
#include "engine/model/silkys.h"
//#include "engine/model/rejet.h"

//#define TEST
#ifdef TEST
# include "engine/model/test.h"
#endif // TEST

AbstractEngine *Engine::getEngine()
{
#ifdef TEST
  if (TestEngine::match()) return new TestEngine;
#endif // TEST
  if (BGIEngine::match()) return new BGIEngine;
  if (EushullyEngine::match()) return new EushullyEngine;
  if (MajiroEngine::match()) return new MajiroEngine;
  if (SiglusEngine::match()) return new SiglusEngine;
  if (SilkysEngine::match()) return new SilkysEngine;
  //if (RejetEngine::match()) return new RejetEngine;
  return nullptr;
}

// EOF
