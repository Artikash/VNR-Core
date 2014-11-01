#ifndef AVRECORDER_H
#define AVRECORDER_H

// avrecorder.h
// 10/28/2014 jichi

#include "sakurakit/skglobal.h"
#include <libavutil/pixfmt.h>
#include <cstdint>

class AVRecorderSettings;
class AVRecorderPrivate;
class AVRecorder
{
  SK_CLASS(AVRecorder)
  SK_DISABLE_COPY(AVRecorder)
  SK_DECLARE_PRIVATE(AVRecorderPrivate)

  // - Construction -
public:
  static bool init();

  explicit AVRecorder();
  ~AVRecorder();
  AVRecorderSettings *settings() const;

  const wchar_t *path() const;
  void setPath(const wchar_t *path);

  bool start();
  bool stop();

  bool addImageData(const uint8_t *data, int64_t size, AVPixelFormat fmt);
  bool addAudioData(const uint8_t *data, int64_t size);
};

#endif // AVRECORDER_H
