// avrecorder.cc
// 9/20/2014 jichi

#include "avrec/avrecorder.h"

#define SK_NO_QT
#define DEBUG "avrecorder.cc"
#include "sakurakit/skdebug.h"

/** Private class */

class AVRecorderPrivate
{
public:
};

/** Public class */

AVRecorder::AVRecorder() : d_(new D) {}
AVRecorder::~AVRecorder() { delete d_; }

// EOF
