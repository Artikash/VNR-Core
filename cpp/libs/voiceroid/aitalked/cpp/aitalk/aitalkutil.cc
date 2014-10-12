// aitalkutil.cc
// 10/11/2014 jichi
#include "aitalk/aitalkutil.h"
#include "aitalk/aitalkapi.h"
#include "aitalk/aitalkconf.h"
#include "cpputil/cpppath.h"
#include <windows.h>
#include <cstring>
#include <iostream>
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
  if (dirlen < 1)
    return;

  ::memcpy(_dirpath, dllpath, dirlen - 1);
  _dirpath[dirlen - 1] = 0;

  ::memcpy(_licpath, dllpath, dirlen);
  ::strcpy(_licpath + dirlen, AITALK_CONFIG_LICENSE);
  std::cerr << _dirpath << std::endl;
  std::cerr << _licpath << std::endl;

  AITalk_TConfig config;
  config.hzVoiceDB = AITALK_CONFIG_FREQ;
  config.msecTimeout = AITALK_CONFIG_TIMEOUT;
  config.dirVoiceDBS = _dirpath; // r'C:\Program Files\AHS\VOICEROID+\zunko'
  config.pathLicense = _licpath; // r'C:\Program Files\AHS\VOICEROID+\zunko\aitalk.lic'
  config.codeAuthSeed = AITALK_CONFIG_CODEAUTHSEED;
  config.lenAuthSeed = AITALK_CONFIG_LENAUTHSEED;

  if (_api.Init(&config) != AITALKERR_SUCCESS)
    return;

  //if (_api.VoiceLoad("zunko_22") != AITALKERR_SUCCESS)
  //  return;

  _valid = true;
}

AITalk::AITalkUtil::~AITalkUtil()
{
  if (_valid)
    _api.End();
}

// Speech synthesize

AITalkResultCode AITalk::AITalkUtil::SynthSync(int *jobID, const AITalk_TJobParam &jobparam, const char *text)
{
  unsigned num;
  AITalk_TTtsParam param[10];
  AITalkResultCode res = _api.GetParam(param, &num);
  if (res != AITALKERR_SUCCESS)
    return res;

  std::cerr << 11111 << std::endl;
  std::cerr << 22222 << param[0].voiceName << 333333 << std::endl;

  //param.procTextBuf = _AITalkProcTextBuf;
  //param.procRawBuf = _AITalkProcRawBuf;
  //param.procEventTts = _AITalkProcEventTTS;
  //res = this->SetParam(param);
  //if (res != AITALKERR_SUCCESS)
  //  return res;

  return _api.TextToSpeech(jobID, &jobparam, text);
}

// EOF
