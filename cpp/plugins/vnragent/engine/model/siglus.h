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
  SiglusEngine() : Base("SiglusEngine", Util::Utf16CodePage, BlockingAttribute) {}

  static bool match();
protected:
  bool attach() override;
  //bool detach() override;
};

// EOF
