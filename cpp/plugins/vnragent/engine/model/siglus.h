#pragma once

// siglus.h
// 5/25/2014 jichi

#include "engine/engine.h"
#include "util/codepage.h"

class SiglusEngine : public AbstractEngine
{
  SK_EXTEND_CLASS(SiglusEngine, AbstractEngine)
  SK_DISABLE_COPY(SiglusEngine)
public:
  static bool match() { return matchFiles("SiglusEngine.exe"); }
  SiglusEngine()  //BlockingAttribute|SingleThreadAttribute|SpecialHookAttribute) // Hook requires restoring the original text after dispatch
  {
    setName("SiglusEngine");
    setWideChar(true);
  }

protected:
  bool attach() override;
  //bool detach() override;
};

// EOF
