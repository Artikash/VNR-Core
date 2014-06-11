#pragma once

// eushully.h
// 6/1/2014 jichi

#include "engine/engine.h"
#include "util/codepage.h"

class EushullyEngine : public AbstractEngine
{
  SK_EXTEND_CLASS(EushullyEngine, AbstractEngine)
  SK_DISABLE_COPY(EushullyEngine)

  static void hookFunction(HookStack *stack);
public:
  EushullyEngine() //BlockingAttribute|SingleThreadAttribute)
  {
    setName("Eushully");
    setWideChar(false);
    setHookFunction(hookFunction);
  }

  // The process name is AGE.EXE.
  // It also contains AGERC.DLL in the game directory.
  static bool match() { return matchFiles("AGERC.DLL"); }

protected:
  bool attach() override;
  //bool detach() override;
};

// EOF
