#pragma once

// test.h
// 5/11/2014 jichi

#include "engine/engine.h"

class BGIEnginePrivate;
class BGIEngine : public AbstractEngine
{
  SK_EXTEND_CLASS(BGIEngine, AbstractEngine)
  SK_DISABLE_COPY(BGIEngine)
  friend class BGIEnginePrivate;
  typedef BGIEnginePrivate D;
public:
  BGIEngine()
    : Base("BGI", SjisEncoding, BlockingAttribute|ExchangeAttribute) {}

  static bool match();
protected:
  bool attach() override;
  bool detach() override;
};

// EOF
