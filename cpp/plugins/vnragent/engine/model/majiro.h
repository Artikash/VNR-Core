#pragma once

// majiro.h
// 4/20/2014 jichi

#include "engine/engine.h"

class MajiroEnginePrivate;
class MajiroEngine : public AbstractEngine
{
  SK_EXTEND_CLASS(MajiroEngine, AbstractEngine)
  SK_DISABLE_COPY(MajiroEngine)
  friend class MajiroEnginePrivate;
  typedef MajiroEnginePrivate D;
public:
  MajiroEngine() : Base("Majiro", "SHIFT-JIS") {}

  static bool match();
  bool inject() override;
};

// EOF
