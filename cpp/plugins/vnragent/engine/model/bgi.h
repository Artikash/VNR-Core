#pragma once

// test.h
// 5/11/2014 jichi

#include "engine/engine.h"
#include "util/codepage.h"

class BGIEngine : public AbstractEngine
{
  SK_EXTEND_CLASS(BGIEngine, AbstractEngine)
  SK_DISABLE_COPY(BGIEngine)
public:
  BGIEngine() : Base("BGI", Util::SjisCodePage, BlockingAttribute) {}

  static bool match();
protected:
  bool attach() override;

private:
  bool attachBGIType1(); // Modified ITH BGI
  bool attachBGIType2(); // VNR BGI2 engine
};

// EOF
