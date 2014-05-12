// engineloader.cc
// 4/26/2014 jichi
// List of all engines

#include "engine/engineloader.h"
//#include "engine/model/bgi.h"
#include "engine/model/majiro.h"

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
  if (MajiroEngine::match()) return new MajiroEngine;
  return nullptr;
}

// EOF
