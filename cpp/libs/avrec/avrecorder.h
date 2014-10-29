#ifndef AVRECORDER_H
#define AVRECORDER_H

// avrecorder.h
// 9/20/2014 jichi

#include "sakurakit/skglobal.h"

class AVRecorderPrivate;
class AVRecorder
{
  SK_CLASS(AVRecorder)
  SK_DISABLE_COPY(AVRecorder)
  SK_DECLARE_PRIVATE(AVRecorderPrivate)

  // - Construction -
public:
  explicit AVRecorder();
  ~AVRecorder();
};

#endif // AVRECORDER_H
