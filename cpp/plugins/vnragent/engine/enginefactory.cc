// enginefactory.cc
// 4/26/2014 jichi
// List of all engines

#include "engine/enginecontroller.h"
#include "engine/enginefactory.h"
#include "windbg/util.h"
#include <boost/foreach.hpp>

#include "engine/model/age.h"
#include "engine/model/aoi.h"
#include "engine/model/bgi.h"
//#include "engine/model/cs2.h"
#include "engine/model/debonosu.h"
#include "engine/model/circus.h"
#include "engine/model/elf.h"
#include "engine/model/retouch.h"
#include "engine/model/rgss.h"
#include "engine/model/siglus.h"
#include "engine/model/silkys.h"
#include "engine/model/system4.h"
//#include "engine/model/unicorn.h"
#include "engine/model/wolf.h"
//#include "engine/model/eushully.h"
//#include "engine/model/majiro.h"
//#include "engine/model/nexas.h"
//#include "engine/model/sideb.h"

#define DEBUG "enginefactory"
#include "sakurakit/skdebug.h"

// TODO: Use boost_pp or boost::mpl to reduce runtime polymorphic
// http://stackoverflow.com/questions/1883862/c-oop-list-of-classes-class-types-and-creating-instances-of-them
EngineController *EngineFactory::createEngine()
{
  static EngineModel *models[] = { // sort reversely
    new WolfRPGEngine
    //, new UnicornEngine
    , new System4Engine
    , new SystemAoiWEngine
    , new SilkysEngine
    , new SiglusEngine
    , new RGSSEngine
    , new RetouchEngine
    , new ElfEngine
    , new DebonosuEngine
    , new CircusEngine
    , new BGIEngine
    , new ARCGameEngine
    //, new ElfEngine
    //, new EushullyEngine
    //, new MajiroEngine
    //, new SideBEngine
  };
  BOOST_FOREACH (EngineModel *m, models) {
    auto p = new EngineController(m);
    if (p->match()) {
      DOUT("matched, engine =" << p->name());
      WinDbg::ThreadsSuspender suspendedThreads; // lock all threads
      if (p->load()) {
        DOUT("ret = true, engine =" << p->name());
        return p;
      }
      DOUT("load failed, engine =" << p->name());
    }
    delete p;
  }
  DOUT("ret = false, none of the engines matches");
  return nullptr;
}

// EOF
