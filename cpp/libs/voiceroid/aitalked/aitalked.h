#ifndef AITALKED_H
#define AITALKED_H

// aitalked.h
// 10/12/2014 jichi
#include "sakurakit/skglobal.h"
#include <string>

class AITalkSynthesizerPrivate;
class AITalkSynthesizer
{
  SK_CLASS(AITalkSynthesizer)
  SK_DISABLE_COPY(AITalkSynthesizer)
  SK_DECLARE_PRIVATE(AITalkSynthesizerPrivate)

public:
  AITalkSynthesizer();
  ~AITalkSynthesizer();

  bool init(const std::wstring &path = L"aitalked.dll");
  bool isValid() const;

  // Settings
  float volume() const;
  void setVolume(float v);

  // Actions

  bool play(const char *text); // it MUST be sjis text
  void stop();
  bool pause();
  bool resume();
};

#endif // AITALKED_H
