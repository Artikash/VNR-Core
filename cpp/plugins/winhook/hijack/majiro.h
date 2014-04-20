#pragma once
// majiro.h
// 4/20/2014 jichi

#include "sakurakit/skglobal.h"

class Engine // abstract
{
  SK_CLASS(Engine)
  SK_DISABLE_COPY(Engine)
public:
  static Self *getEngine();

  static bool isEnabled(bool t);
  static void setEnabled(bool t);

  Enine() {}
  virtual ~Engine() {}
  virtual bool inject() = 0;
};

class Majiro : public Engine
{
  SK_EXTEND_CLASS(Majiro, Engine)
  SK_DISABLE_COPY(Majiro)
public:
  static bool match();
  bool inject() override;
};

// EOF
