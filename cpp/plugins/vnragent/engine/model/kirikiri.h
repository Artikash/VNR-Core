#pragma once

// kirikiri.h
// 5/10/2014 jichi
// TODO: Incompleted

#include "engine/engine.h"

class KiriKiriEnginePrivate;
class KiriKiriEngine : public AbstractEngine
{
  SK_EXTEND_CLASS(KiriKiriEngine, AbstractEngine)
  SK_DISABLE_COPY(KiriKiriEngine)
  friend class KiriKiriEnginePrivate;
  typedef KiriKiriEnginePrivate D;
public:
  KiriKiriEngine() : Base("KiriKiri", "UTF-16") {}

  static bool match();
  bool inject() override;
  bool unload() override;
};

// EOF
