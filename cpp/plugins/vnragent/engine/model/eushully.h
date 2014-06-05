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
  EushullyEngine()
    : Base("Eushully", Util::SjisCodePage, BlockingAttribute|SingleThreadAttribute)
  { setHookFunction(hookFunction); }

  static bool match();
protected:
  bool attach() override;
  //bool detach() override;
};

// EOF
