// aitalked.cc
// 10/12/2014 jichi
#include "aitalked/aitalked.h"
#include "aitalk/aitalkutil.h"
#include <windows.h>

using namespace AITalk;

/** Private class */

class AITalkSynthesizerPrivate
{
public:
  HMODULE h;
  int jobId;
  AITalkUtil ai;

  AITalkSynthesizerPrivate() : h(nullptr), jobId(0) {}
};

/** Public class */

AITalkSynthesizer::AITalkSynthesizer()
  : d_(new D)
{}

AITalkSynthesizer::~AITalkSynthesizer()
{
  HMODULE h = d_->h;
  delete d_;
  if (h)
    ::FreeLibrary(h);
}

bool AITalkSynthesizer::isValid() const { return d_->ai.IsValid(); }

bool AITalkSynthesizer::init(const std::wstring &path)
{
  if (d_->h)
    ::FreeLibrary(d_->h);
  d_->h = LoadLibraryW(path.c_str());
  return d_->h && d_->ai.Init(d_->h);
}

// Settings

// Actions

bool AITalkSynthesizer::pause()
{ return d_->ai.IsValid() && d_->ai.Suspend() == 0; }

bool AITalkSynthesizer::resume()
{ return d_->ai.IsValid() && d_->ai.Resume() == 0; }

void AITalkSynthesizer::stop()
{
  if (d_->ai.IsValid() && d_->ai.IsSynthesizing() && d_->jobId) {
    d_->ai.CloseSpeech(d_->jobId);
    d_->jobId = 0;
  }
}

bool AITalkSynthesizer::play(const std::string &text)
{
  if (!d_->ai.IsValid())
    return false;
  if (d_->ai.IsSynthesizing() && d_->jobId)
    d_->ai.CloseSpeech(d_->jobId);
  d_->jobId = 0;
  return d_->ai.TextToSpeech(&d_->jobId, text.c_str()) == 0;
}

// EOF
