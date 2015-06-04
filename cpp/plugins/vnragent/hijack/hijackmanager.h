#pragma once

// hijackmanager.h
// 6/3/2015 jichi

#include "sakurakit/skglobal.h"

class HijackManagerPrivate;
class HijackManager
{
  SK_CLASS(HijackManager)
  SK_DISABLE_COPY(HijackManager)
  SK_DECLARE_PRIVATE(HijackManagerPrivate)

public:
  static Self *instance(); // needed by Engine

  HijackManager();
  ~HijackManager();

  bool isFunctionAttached(unsigned long addr) const;
  void attachFunction(unsigned long addr);
  void detachFunction(unsigned long addr);
};

// EOF
