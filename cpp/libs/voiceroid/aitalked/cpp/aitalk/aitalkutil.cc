// aitalkutil.cc
// 10/11/2014 jichi
#include "aitalk/aitalkutil.h"
#include "aitalk/aitalkapi.h"
#include "aitalk/aitalkconf.h"
#include "cpputil/cpppath.h"
#include <windows.h>
#include <cstring>

using namespace AITalk;

// Construction

AITalk::AITalkUtil::AITalkUtil(HMODULE h)
  : _api(h), _valid(false)
{
  if (!_api.IsValid())
    return;

  char dllpath[MAX_PATH];
  if (!::GetModuleFileNameA(h, dllpath, MAX_PATH))
    return;

  size_t dirlen = ::cpp_wdirlen(dllpath);
  if (!dirlen)
    return;

  char dirpath[MAX_PATH]; // path to zunko
  ::memcpy(dirpath, dllpath, dirlen - 1);
  dirpath[dirlen] = 0;

  char licpath[MAX_PATH]; // path to aitalk.lic
  ::memcpy(licpath, dllpath, dirlen);
  ::strcpy(licpath + dirlen, AITALK_CONFIG_LICENSE);

  AITalk_TConfig config;
  config.hzVoiceDB = AITALK_CONFIG_FREQ;
  config.msecTimeout = AITALK_CONFIG_TIMEOUT;
  config.dirVoiceDBS = dirpath; // r'C:\Program Files\AHS\VOICEROID+\zunko'
  config.pathLicense = licpath; // r'C:\Program Files\AHS\VOICEROID+\zunko\aitalk.lic'
  config.codeAuthSeed = AITALK_CONFIG_CODEAUTHSEED;
  config.lenAuthSeed = AITALK_CONFIG_LENAUTHSEED;

  _valid = _api.Init(&config) == AITALKERR_SUCCESS;
}

AITalk::AITalkUtil::~AITalkUtil()
{
  if (_valid)
    _api.End();
}

// Speech synthesize

AITalkResultCode AITalk::AITalkUtil::SynthSync(int *jobID, const AITalk_TJobParam &jobparam, const char *text)
{
  AITalk_TTtsParam param;
  AITalkResultCode res = this->GetParam(&param);
  if (res != AITALKERR_SUCCESS)
    return res;

  //param.procTextBuf = _AITalkProcTextBuf;
  //param.procRawBuf = _AITalkProcRawBuf;
  //param.procEventTts = _AITalkProcEventTTS;
  res = this->SetParam(param);
  if (res != AITALKERR_SUCCESS)
    return res;

  return _api.TextToSpeech(jobID, &jobparam, text);
}

// EOF
