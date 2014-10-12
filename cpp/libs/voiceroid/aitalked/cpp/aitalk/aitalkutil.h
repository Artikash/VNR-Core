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
  // AITalkUtil.cs: public AITalkResultCode GetStatus(int jobID, out AITalkStatusCode status)
  AITalkResultCode GetStatus(int jobID, _Out_ AITalkStatusCode *status) const
  { return _api.GetStatus(jobID, status); }

  // AITalkUtil.cs: public AITalkResultCode SetParam(ref AITalk_TTtsParam param)
  AITalkResultCode SetParam(const AITalk_TTtsParam &param)
  { return _api.SetParam(&param); }

  // AITalkUtil.cs: public virtual AITalkResultCode SynthAsync(ref AITalk_TJobParam jobparam, string text)
  AITalkResultCode SynthSync(_Out_ int *jobID, const AITalk_TJobParam &jobparam, const char *text);

  // AITalkUtil.cs: public AITalkResultCode GetParam(ref AITalk_TTtsParam param)
  AITalkResultCode GetParam(_Out_ AITalk_TTtsParam *param)
  {
    unsigned int num;
    return _api.GetParam(param, &num);
  }

public:
  // AITalkUtil::SynthSync: while ((this._playing && (res == AITalkResultCode.AITALKERR_SUCCESS)) && (code != AITalkStatusCode.AITALKSTAT_DONE));
  bool IsPlaying(int jobID) const
  {
    AITalkStatusCode code;
    AITalkResultCode res = this->GetStatus(jobID, &code);
    return res == AITALKERR_SUCCESS && code == AITALKSTAT_DONE;
  }

  // AITalkUtil.cs: public AITalkResultCode TextToSpeech(string text)
  AITalkResultCode TextToSpeech(_Out_ int *jobID, const char *text)
  {
     AITalk_TJobParam param;
     param.modeInOut = AITALKIOMODE_PLAIN_TO_WAVE;
     param.userData = nullptr;
     return this->SynthSync(jobID, param, text);
  }

};

} // namespace AITalk
