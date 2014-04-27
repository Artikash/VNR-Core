#pragma once

// engine.h
// 4/20/2014 jichi

#include "sakurakit/skglobal.h"

class AbstractEngine
{
  SK_CLASS(AbstractEngine)
  SK_DISABLE_COPY(AbstractEngine)
public:
  static Self *getEngine();

  AbstractEngine() {}
  virtual ~AbstractEngine() {}
  virtual bool inject() = 0;

  //static bool isEnabled();
  //static void setEnabled(bool t);
};


// EOF
