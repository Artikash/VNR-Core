// enginefactory.cc
// 4/26/2014 jichi
// List of all engines

#include "engine/enginecontroller.h"
#include "engine/enginefactory.h"
#include "engine/model/bgi.h"
#include "engine/model/circus.h"
#include "engine/model/elf.h"
#include "engine/model/eushully.h"
#include "engine/model/majiro.h"
#include "engine/model/siglus.h"
#include <boost/foreach.hpp>

// TODO: Use boost_pp or boost::mpl to reduce runtime polymorphic
// http://stackoverflow.com/questions/1883862/c-oop-list-of-classes-class-types-and-creating-instances-of-them
EngineController *EngineFactory::createEngine()
{
  static EngineModel *models[] = {
   new BGIEngine
   , new CircusEngine
   , new ElfEngine
   , new EushullyEngine
   , new MajiroEngine
   , new SiglusEngine
  };
  BOOST_FOREACH (EngineModel *m, models) {
    auto p = new EngineController(m);
    if (p->match() && p->load())
      return p;
    delete p;
  }
  return nullptr;
}

// EOF
