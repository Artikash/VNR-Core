#pragma once

// driver.h
// 2/1/2013 jichi

#include "sakurakit/skglobal.h"

class DriverPrivate;
// Root object for all qobject
class Driver
{
  SK_CLASS(Driver)
  SK_DISABLE_COPY(Driver)
  SK_DECLARE_PRIVATE(DriverPrivate)
public:
  Driver();
  ~Driver();

  //void quit();
};

// EOF
