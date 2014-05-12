#pragma once

// test.h
// 5/11/2014 jichi

#include "engine/engine.h"

class TestEnginePrivate;
class TestEngine : public AbstractEngine
{
  SK_EXTEND_CLASS(TestEngine, AbstractEngine)
  SK_DISABLE_COPY(TestEngine)
  friend class TestEnginePrivate;
  typedef TestEnginePrivate D;
public:
  TestEngine() : Base("Test", SjisEncoding,
      BlockingAttribute|ExchangeAttribute) {}

  static bool match();
protected:
  bool attach() override;
  bool detach() override;
};

// EOF
