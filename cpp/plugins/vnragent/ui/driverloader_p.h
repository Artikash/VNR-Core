#pragma once

// main_p.h
// 1/31/2013 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QTimer>

class MainPrivate
{
  SK_CLASS(MainPrivate)
  SK_DISABLE_COPY(MainPrivate)

  QTimer *retransTimer; // refresh windows
  QTimer *rehookTimer;  // refresh modules

public:
  MainPrivate();
  ~MainPrivate();
};

// EOF
