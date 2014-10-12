#pragma once

// aitalkutil.h
// 10/11/2014 jichi
#include "aitalk/aitalkapi.h"

namespace AITalk {

// C#: public class AITalkUtil : IDisposable
class AITalkUtil
{
  AITalkAPI _api;
  bool _valid;
public:
  /**
   *  @param  h  aitalked.dll module
   *
   *  Create Config and invoke AITalkAPI::Init.
   */
  explicit AITalkUtil(HMODULE h);

  /**
   *  Invoke AITalk::End.
   */
  ~AITalkUtil();

  bool IsValid() const { return _valid; }

private:
  // C#: public virtual AITalkResultCode SynthAsync(ref AITalk_TJobParam jobparam, string text)
  AITalkResultCode SynthSync(_Out_ int *jobID, const AITalk_TJobParam &jobparam, const char *text);

public:

  // C#: public AITalkResultCode GetStatus(int jobID, out AITalkStatusCode status)
  AITalkResultCode GetStatus(int jobID, _Out_ AITalkStatusCode *status) const
  { return _api.GetStatus(jobID, status); }

  // C#: public AITalkResultCode TextToSpeech(string text)
  AITalkResultCode TextToSpeech(_Out_ int *jobID, const char *text)
  {
     AITalk_TJobParam param;
     param.modeInOut = AITALKIOMODE_PLAIN_TO_WAVE;
     param.userData = nullptr;
     return SynthSync(jobID, param, text);
  }

};

} // namespace AITalk
