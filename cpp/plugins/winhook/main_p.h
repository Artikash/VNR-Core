#pragma once

// main_p.h
// 1/31/2013 jichi

#include "sakurakit/skglobal.h"
#include "wintimer/wintimer.h"

class MainPrivate
{
  SK_CLASS(MainPrivate)
  SK_DISABLE_COPY(MainPrivate)

  WinTimer retransTimer; // refresh windows
  WinTimer rehookTimer;  // refresh modules

public:
  MainPrivate();
  ~MainPrivate();
};

// EOF
