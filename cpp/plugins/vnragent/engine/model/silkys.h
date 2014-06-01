#pragma once

// silkys.h
// 5/31/2014 jichi

#include "engine/engine.h"
#include "util/codepage.h"

class SilkysEngine : public AbstractEngine
{
  SK_EXTEND_CLASS(SilkysEngine, AbstractEngine)
  SK_DISABLE_COPY(SilkysEngine)
public:
  SilkysEngine() : Base("Silkys", Util::SjisCodePage, BlockingAttribute) {}

  static bool match();
protected:
  bool attach() override;
  //bool detach() override;
};

// EOF
