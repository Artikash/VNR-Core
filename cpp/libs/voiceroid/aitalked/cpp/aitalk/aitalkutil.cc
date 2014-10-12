// aitalkutil.cc
// 10/11/2014 jichi
#include "aitalk/aitalkutil.h"
#include "aitalk/aitalkapi.h"
#include "aitalk/aitalkmarshal.h"
#include "cpputil/cpppath.h"
#include <windows.h>
#include <cstring>

using namespace AITalk;

// Construction

AITalk::AITalkUtil::AITalkUtil(HMODULE h)
  : _talk(h), _audio(h), _valid(false)
{
  if (!_talk.IsValid() || !_audio.IsValid())
    return;

  // Initialize talk API

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

  {
    AITalk_TConfig config;
    config.hzVoiceDB = AITALK_CONFIG_FREQUENCY;
    config.msecTimeout = AITALK_CONFIG_TIMEOUT;
    //config.dirVoiceDBS = _dirpath; // r'C:\Program Files\AHS\VOICEROID+\zunko'
    config.dirVoiceDBS = "C:\\Program Files\\AHS\\VOICEROID+\\zunko\\voice";
    //config.pathLicense = _licpath; // r'C:\Program Files\AHS\VOICEROID+\zunko\aitalk.lic'
    config.pathLicense = "C:\\Program Files\\AHS\\VOICEROID+\\zunko\\aitalk.lic";
    config.codeAuthSeed = AITALK_CONFIG_CODEAUTHSEED;
    config.lenAuthSeed = AITALK_CONFIG_LENAUTHSEED;

    if (_talk.Init(&config) != AITALKERR_SUCCESS)
      return;
  }

  AITalkResultCode code;
  static char *langDir =  "C:\\Program Files\\AHS\\VOICEROID+\\zunko\\lang";
  code = _talk.LangLoad(langDir);
  if (code != AITALKERR_SUCCESS)
    return;

  static char *voiceName = "zunko_22";
  code = _talk.VoiceLoad(voiceName);
  if (code != AITALKERR_SUCCESS)
    return;

  // Initialize audio API
  {
    AIAudio_TConfig config;
    config.msecLatency = AIAUDIO_CONFIG_BUFFERLATENCY;
    config.lenBufferBytes = AITALK_CONFIG_FREQUENCY * 2 * AIAUDIO_CONFIG_BUFFERLENGTH;
    config.hzSamplesPerSec = AITALK_CONFIG_FREQUENCY;
    config.formatTag = AIAUDIOTYPE_PCM_16;
    config.__reserved__ = 0;
    if (_audio.Open(&config) != AIAUDIOERR_SUCCESS)
      return;
  }

  _valid = true;
}

AITalk::AITalkUtil::~AITalkUtil()
{
  if (_valid) {
    _audio.Close();
    _talk.End();
  }
}

// Speech synthesize

AITalkResultCode AITalk::AITalkUtil::SynthSync(int *jobID, const AITalk_TJobParam &jobparam, const char *text)
{
  /*
  //unsigned num;
  //AITalk_TTtsParam param[10];
  //AITalkResultCode res = _talk.GetParam(param, &num);
  //if (res != AITALKERR_SUCCESS)
  //  return res;

  //param.procTextBuf = _AITalkProcTextBuf;
  //param.procRawBuf = _AITalkProcRawBuf;
  //param.procEventTts = _AITalkProcEventTTS;
  //res = this->SetParam(param);
  //if (res != AITALKERR_SUCCESS)
  //  return res;

  size_t num;
  AITalkResultCode code = _talk.GetParam(nullptr, &num);
  if (code != AITALKERR_INSUFFICIENT)
    return AITALKERR_INSUFFICIENT;

  char *data = new char[num];
  ((int *)data)[0] = num;
  //IntPtr ptr = Marshal.AllocCoTaskMem((int) num);
  code = _talk.GetParam(data, &num);
  if (code != AITALKERR_SUCCESS) {
    delete data;
    return code;
  }
  //param = AITalkMarshal.IntPtrToTTtsParam(ptr);
  //Marshal.FreeCoTaskMem(ptr);
  */

  //AITalk_TTtsParam param;
  //AITalkMarshal::ReadTtsParam(&param, data);
  return _talk.TextToSpeech(jobID, &jobparam, text);
}

// EOF
